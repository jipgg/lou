declare print: <Tostring_Compatible_Args...>(Tostring_Compatible_Args...) -> ()
declare warn: <Tostring_Compatible_Args...>(Tostring_Compatible_Args...) -> ()
declare class Rect
    x: number
    y: number
    w: number
    h: number
    function as_tuple(self): (number, number, number, number)
end
declare class Point
    x: number
    y: number
    function as_tuple(self): (number, number)
end

declare class Color
    r: number
    g: number
    b: number
    a: number
    function as_tuple(self): (number, number, number, number)
end

declare class Lou_Console
    print: <Tostring_Compatible_Args...>(self: Lou_Console, Tostring_Compatible_Args...) -> ()
    warn: <Tostring_Compatible_Args...>(self: Lou_Console, Tostring_Compatible_Args...) -> ()
    error: <Tostring_Compatible_Args...>(self: Lou_Console, Tostring_Compatible_Args...) -> ()
end

declare class Lou_Window
    function position(self): (number, number)
    function size(self): (number, number)
    function resize(self, width: number, height: number): ()
    function reposition(self, x: number, y: number): ()
    function opacity(self): number
    function set_opacity(self, opacity: number): ()
    function title(self): string
    function set_title(self, title: string): ()
    function maximize(self): ()
    function minimize(self): ()
    function restore(self): ()
    function enable_fullscreen(self, fullscreen: boolean): ()
    function size_in_pixels(self): (number, number)
    function aspect_ratio(self): (number, number)

end
declare class Lou_Renderer
    function draw_rect(self, rect: Rect): ()
    function fill_rect(self, rect: Rect): ()
    function draw_point(self, x: number, y: number): ()
    function draw_line(self, from: Point, to: Point): ()
    function set_draw_color(self, r: number, g: number, b: number, a: number?): ()
    function clear(self): ()
end

declare class Lou_Mouse
    function position(self): (number, number)
    function pressed(self, handler: (type: ('left'|'middle'|'right'), x: number, y: number)->()): ()
    function released(self, handler: (type: ('left'|'middle'|'right'), x: number, y: number)->()): ()
    function moved(self, handler: (x: number, y: number)->()): ()
end
declare class Lou_Keyboard
    function pressed(self, handler: (key: string)->()): ()
    function released(self, handler: (key: string)->()): ()
    function is_pressed(self, key: string): boolean
end

declare class Lou_State 
    renderer: Lou_Renderer
    window: Lou_Window
    mouse: Lou_Mouse
    get_mouse: () -> Lou_Mouse
    keyboard: Lou_Keyboard
    console: Lou_Console
    function on_render(self, fn: ()->()): ()
    function on_update(self, fn: (delta_seconds: number)->()): ()
end

declare lou: Lou_State 
declare function Rect(x: number?, y: number?, w: number?, h: number?): Rect
declare function Point(x: number?, y: number?): Point
declare function Color(r: number?, g: number?, b: number?, a: number?): Color
