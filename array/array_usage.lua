
package.cpath = "./?.so;D:/workspace/visual-studio/2017/repos/LuaIntegration/Debug/?.dll"
require "arrayoop"

local a = arrayoop.new(2)

for i = 1, 2 do
	a:set(i, i % 2 == 0)
end

print("a[1]=", a:get(1))
print("a[2]=", a:get(2))
print("size=", a:size())

print(a)