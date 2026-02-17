---
-- @file src/macros.lua
-- @brief Macro expansion and compile-time evaluation engine.
--
-- This module processes the raw AST produced by the parser.
-- Its responsibilities include:
-- 1. Defining macros (`defmacro`) and storing them.
-- 2. Expanding macro calls into their AST replacements.
-- 3. Evaluating compile-time Lua blocks (`(lua ...)`).
-- 4. Loading plugins (`require`), stylesheets (`style`), and modules (`import`).
-- 5. Implementing file inclusion (`include`).
-- 6. Creating a sandboxed Lua environment for compile-time execution.
---

-- local loader = require("loader")
local parser = require("parser")
local gui = require("gui_xt") -- Required for style loading
local M = {}
local macros = {}
local loaded_plugins = {}
local loaded_modules = {}

local function inspect(t) -- Simple inspect function for debugging
    if type(t) ~= "table" then
        return tostring(t)
    end
    local function _inspect(t, indent)
        local s = ""
        for k, v in pairs(t) do
            s = s .. indent
            if type(k) == "string" then
                s = s .. k .. " = "
            end
            if type(v) == "table" then
                s = s .. "{\n" .. _inspect(v, indent .. "  ") .. indent .. "}\n"
            else
                s = s .. tostring(v) .. "\n"
            end
        end
        return s
    end
    return "{\n" .. _inspect(t, "  ") .. "}"
end

local function read_file(path)
    local f = io.open(path, "r")
    if not f then error("Could not open file: " .. path) end
    local content = f:read("*a")
    f:close()
    return content
end

local function create_sandbox()
  local env = {
    -- Constants
    screenwidth = 1920,
    screenheight = 1080,
    dpi = 96,
    charwidth = 8,
    charheight = 16,
    platform = "linux",
    
    -- Modules
    gui = require("gui_xt"),
    plugins = loaded_plugins,
    
    -- Safe Lua basics
    _G = _G, -- Allow access to globals
    io = io, -- Allow IO access (popen)
    math = math,
    table = table,
    string = string,
    pairs = pairs,
    ipairs = ipairs,
    next = next,
    tonumber = tonumber,
    tostring = tostring,
    type = type,
    select = select,
    error = error,
    assert = assert,
    print = print,
    pcall = pcall,
    xpcall = xpcall,
    load = load,
    os = {
        date = os.date,
        time = os.time,
        clock = os.clock,
        difftime = os.difftime,
        execute = os.execute,
        exit = os.exit,
        getenv = os.getenv
    },
  }
  
  setmetatable(env, { 
    __index = function(t, k) 
        if loaded_plugins[k] then return loaded_plugins[k] end
        if loaded_modules[k] then return loaded_modules[k] end
        if k == "running" then return _G.running end
        return nil 
    end,
    __newindex = function(t, k, v)
        if k == "running" then _G.running = v 
        else rawset(t, k, v) end
    end
  })
  return env
end

local function eval_lua(code, local_env)
  local env = create_sandbox()
  
  local echo_result = nil
  env.luiecho = function(text)
      local ast = parser.parse(text)
      if ast then
          echo_result = ast
      else
          error("luiecho: Failed to parse string: " .. tostring(text))
      end
  end
  
  if local_env then
    for k, v in pairs(local_env) do
      env[k] = v
    end
  end

  local f, err = load(code, "lua-block", "t", env)
  if not f then
    error("Compile-Time Lua error: " .. tostring(err))
  end
  local ok, result = pcall(f)
  if not ok then
    error("Compile-Time Lua execution error: " .. tostring(result))
  end
  
  if echo_result then
      return echo_result
  end
  
  return result
end

local function expand_node(node, env)
  if type(node) == "table" then
    if node.defmacro then
      return node
    elseif node.keyword then
      return node
    elseif node.lua then
       return eval_lua(node.lua, env)
    elseif node[1] == "include" and type(node[2]) == "string" then
       local content = read_file(node[2])
       local included_ast = parser.parse(content)
       if not included_ast then error("Failed to parse included file: " .. node[2]) end
       
       local expanded_included = {}
       for _, child in ipairs(included_ast) do
           table.insert(expanded_included, expand_node(child, env))
       end
       return expanded_included
       
    elseif node[1] and type(node[1]) == "string" and macros[node[1]] then
      -- Macro Call
      local macro_name = node[1]
      local macro_def = macros[macro_name]
      local macro_args = macro_def.args
      local macro_body = macro_def.body

      local call_values = {}
      for i = 2, #node do
        table.insert(call_values, expand_node(node[i], env))
      end

      local current_env = {}
      for k, v in pairs(env) do
        current_env[k] = v
      end

      for i, arg_name in ipairs(macro_args) do
        current_env[arg_name] = call_values[i]
      end
      
      local expanded_body = {}
      for _, body_node in ipairs(macro_body) do
        table.insert(expanded_body, expand_node(body_node, current_env))
      end
      
      if #expanded_body == 1 then
        return expanded_body[1]
      else
        return expanded_body
      end

    else -- Regular list or generic table
      local new_node = {}
      local is_list = true
      for k, v in pairs(node) do
        if type(k) ~= "number" then is_list = false end
        new_node[k] = expand_node(v, env)
      end

      if not is_list then return new_node end

      -- Special handling for lists (splicing)
      local spliced_node = {}
      for i = 1, #new_node do
        local expanded_v = new_node[i]
        
        local function is_ast_node(t)
            if type(t) ~= "table" then return false end
            if type(t[1]) == "string" then return true end
            if t.defmacro or t.keyword or t.lua or t.require or t.import or t.style then return true end
            return false
        end

        local should_splice = false
        if type(expanded_v) == "table" then
             if #expanded_v > 0 and is_ast_node(expanded_v[1]) then
                 should_splice = true
             end
        end

        if should_splice then
          for _, item in ipairs(expanded_v) do
            table.insert(spliced_node, item)
          end
        else
          table.insert(spliced_node, expanded_v)
        end
      end
      return spliced_node
    end
  elseif type(node) == "string" and env[node] then
    return env[node]
  end

  return node
end

local function macro_expand(ast_raw)
  local initial_ast = {}
  for _, node in ipairs(ast_raw) do
    if type(node) == "table" and node.defmacro then
      macros[node.defmacro] = { args = node.args, body = node.body }
    elseif type(node) == "table" and node.require then
      -- local plugin = loader.load(node.require, node.version)
      -- if plugin then
      --     loaded_plugins[node.require] = plugin
      -- end
    elseif type(node) == "table" and node[1] == "style" and type(node[2]) == "string" then
       gui.load_css(node[2])
    elseif type(node) == "table" and node[1] == "import" and type(node[2]) == "string" then
       local mod_name = node[2]
       local ok, mod = pcall(require, mod_name)
       if not ok then
           error("Failed to import module '" .. mod_name .. "': " .. tostring(mod))
       end
       loaded_modules[mod_name] = mod
    else
      table.insert(initial_ast, node)
    end
  end

  local expanded_ast = {}
  for _, node in ipairs(initial_ast) do
    local result = expand_node(node, {})
    
    local is_list = type(result) == "table" and (type(result[1]) == "table" or #result == 0)
    local is_node = type(result) == "table" and type(result[1]) == "string"
    local is_special = type(result) == "table" and (result.defmacro or result.keyword)

    if is_list and not is_special and not is_node then
       for _, item in ipairs(result) do
         table.insert(expanded_ast, item)
       end
    else
       table.insert(expanded_ast, result)
    end
  end

  return expanded_ast
end

function M.register(name, args, body)

    macros[name] = { args = args, body = body }

end



return { expand = macro_expand, register = M.register }
