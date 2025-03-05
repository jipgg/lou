local a = Vec2(100, 200)
local b = Vec2(200, -100)
print(a + b)
print(a - b)
print(a * b)
print(a:dot(b))
print(a:dot(a))
print(a:normalized())
print(a:length())
print(a:squared_length())
print(a:normalized():length())

return {
    a = a,
}
