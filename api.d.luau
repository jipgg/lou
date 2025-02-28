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

declare class Lou_Renderer
    scale: number
    color_scale: number
    draw_color: Color
    viewport: Rect
    clip_rect: Rect
    vsync_enabled: boolean
    function draw_rect(self, rect: Rect): ()
    function draw_point(self, point: Point): ()
    function draw_line(self, from: Point, to: Point): ()
    function draw_triangle(self, a: Point, b: Point, c: Point): ()
    function draw_ellipse(self, center: Point, radius_x: number, radius_y: number): ()
    function clear(self): ()
end
declare class Lou_Console
    print: <Tostring_Compatible_Args...>(self: Lou_Console, Tostring_Compatible_Args...) -> ()
    warn: <Tostring_Compatible_Args...>(self: Lou_Console, Tostring_Compatible_Args...) -> ()
    error: <Tostring_Compatible_Args...>(self: Lou_Console, Tostring_Compatible_Args...) -> ()
end

declare class Lou_Window
    x: number
    y: number
    w: number
    h: number
    opacity: number
    border: boolean
end

declare class Lou_Mouse
    --x: number
    --y: number
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

declare class Lou_Engine 
    renderer: Lou_Renderer
    window: Lou_Window
    mouse: Lou_Mouse
    get_mouse: () -> Lou_Mouse
    keyboard: Lou_Keyboard
    console: Lou_Console
    function on_draw(self, fn: ()->()): ()
    function on_update(self, fn: (delta_seconds: number)->()): ()
end

declare lou: Lou_Engine 
declare function Rect(x: number?, y: number?, w: number?, h: number?): Rect
declare function Point(x: number?, y: number?): Point
declare function Color(r: number?, g: number?, b: number?, a: number?): Color
