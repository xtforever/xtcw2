% ------------------------------------------------------------------------------------
% -- to demonstrate, we can define some constants or functions 
% -- which can be used later in the precompiler sections 
% ------------------------------------------------------------------------------------
<?LUA

-- generate lua code to be parsed in the 2nd parser stage 
function lua_var_export(key,val)
	print( "%%LUA\n" .. key .. "=" .. val .. "\n%%END" )
end

num_edit_widgets=8
?>


%%
%% set default colors
%%
*allowShellResize: true

*foreground: #1cf0f1
*background: #ffffff
*Gridbox.background: Lightblue

<?LUA

lua_var_export( "WidCNT", num_edit_widgets )


?>
