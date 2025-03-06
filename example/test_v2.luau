local b = vector.create(200, -100, 0, 0)
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
