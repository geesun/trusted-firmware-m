-- This tool is used to fix tfm_s.dir/build.make file


local f
local data

local path = '.\\app\\secure_fw\\CMakeFiles\\tfm_s.dir\\build.make' 

print("fix build file:", path)

f = io.open(path, 'rt')
if(f ~= nil)then
    data = f:read("*all")
    f:close()
else
    print("Cannot open file")
    os.exit()
end

-- app/secure_fw/tfm_s.axf: app/secure_fw/CMakeFiles/tfm_s.dir/objects1.rsp
-- @CMakeFiles/tfm_s.dir/objects1.rsp

data = string.gsub(data, "@CMakeFiles/tfm_s%.dir/objects1%.rsp", "$(tfm_s_EXTERNAL_OBJECTS)")
data = string.gsub(data, "app/secure_fw/tfm_s%.axf: app/secure_fw/CMakeFiles/tfm_s%.dir/objects1%.rsp", "")


f = io.open(path, 'wt')
if(f ~= nil)then
    f:write(data)
    f:close()
else
    print("Cannot write file");
    os.exit()    
end
print("done")