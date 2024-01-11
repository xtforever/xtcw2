p=edit-test
make &&
cpp <$p.adp | sed "s/\\\\/\n/g" >pre.adp &&
./adpconv.lua $p.adp $p.ad $p.lua &&
./luarunner -TraceLevel 1 -Luafile $p.lua -xrm "*WclResFiles:${p}.ad"

