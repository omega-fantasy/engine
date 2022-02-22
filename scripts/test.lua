printbox("Hello World!")

local x = 10
local y = 5
local z = x + y
printbox("10 + 5 is " .. tostring(z))

local k = math.log(10000)
printbox("log of 10000 is " .. tostring(k))

local l = math.sqrt(2)
printbox("square root of 2 is " .. tostring(l))

function myfunction(t)
    print(t[1], t[2])
    print("Hello World")
    return { 1, 2 }
end

HUD_add_button("mybutton", "myfunction", {15, 20})