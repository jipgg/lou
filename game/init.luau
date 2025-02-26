print('Hello')

game:print("YO ")


function game.key_down(key: Key_Name)
    if key == 'M' then
        game:print("yoo m is pressed")
    end
end

function game.key_up(key)
    game:print(`key up {key}`)
end
