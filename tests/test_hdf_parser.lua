-- Unit test for tools/hdf.lua (core HDF + seq + rem).
package.path = "tools/?.lua;" .. package.path
local hdf = require("hdf")

local d = hdf.to_data(hdf.parse_text([=[
(rem "header")
(
  a 1
  b (seq 2 3)
  c [[raw text]]
  d "quoted\ntext"
  e %
  f true
  g false
  h null
)
]=]))
assert(d.a == 1, "number")
assert(type(d.b) == "table" and d.b[1] == 2 and d.b[2] == 3, "seq")
assert(d.c == "raw text", "raw string")
assert(d.d == "quoted\ntext", "quoted escape")
assert(d.e == "", "empty")
assert(d.f == true, "true")
assert(d.g == false, "false")
assert(d.h == nil, "null")

-- Raw strings with an equals delimiter and a trailing bracket.
local d2 = hdf.to_data(hdf.parse_text([==[
(value [=[a [b] c]=] trailing [=[ends with ]]=])
]==]))
assert(d2.value == "a [b] c", "equals raw string")
assert(d2.trailing == "ends with ]", "trailing bracket")

-- rem forms are ignored inside objects and seq arrays.
local d3 = hdf.to_data(hdf.parse_text([[
(seq 1 (rem ignored) 2)
]]))
assert(#d3 == 2 and d3[1] == 1 and d3[2] == 2, "rem in seq")

print("test_hdf_parser passed")
