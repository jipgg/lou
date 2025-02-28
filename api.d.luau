declare print: <Tostring_Compatible_Args...>(Tostring_Compatible_Args...) -> ()
declare class Rect
    x: number
    y: number
    w: number
    h: number
end
declare class Point
    x: number
    y: number
end

declare class Color
    r: number
    g: number
    b: number
    a: number
end

declare class Renderer
    scale: number
    color_scale: number
    draw_color: Color
    viewport: Rect
    clip_rect: Rect
    vsync_on: boolean

    function draw_rect(self, rect: Rect): ()
    function draw_point(self, point: Point): ()
    function draw_line(self, from: Point, to: Point): ()
    function set_draw_color(self, color: Color): ()
    function clear(self): ()
end
declare class Console
    print: <Tostring_Compatible_Args...>(self: Console, Tostring_Compatible_Args...) -> ()
    warn: <Tostring_Compatible_Args...>(self: Console, Tostring_Compatible_Args...) -> ()
    error: <Tostring_Compatible_Args...>(self: Console, Tostring_Compatible_Args...) -> ()
end

declare class Window
    x: number
    y: number
    width: number
    height: number
    opacity: number
    border: boolean
    function set_size(self, w: number, h: number): ()
    function get_size(self): (number, number)
    function set_position(self, x: number, y: number): ()
    function get_position(self): (number, number)
end

declare class Engine 
    renderer: Renderer
    window: Window
    console: Console
end

declare lou: Engine 
declare function Rect(x: number?, y: number?, w: number?, h: number?): Rect
declare function Point(x: number?, y: number?): Point
declare function Color(r: number?, g: number?, b: number?, a: number?): Color
