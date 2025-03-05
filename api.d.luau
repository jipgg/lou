declare print: <Tostring_Compatible_Args...>(Tostring_Compatible_Args...) -> ()
declare warn: <Tostring_Compatible_Args...>(Tostring_Compatible_Args...) -> ()
declare class Lou_Rect
    x: number
    y: number
    w: number
    h: number
    function as_tuple(self): (number, number, number, number)
end
declare class Lou_Vec2
    x: number
    y: number
    function as_tuple(self): (number, number)
    function __add(self, other: Lou_Vec2): Lou_Vec2
    function __sub(self, other: Lou_Vec2): Lou_Vec2
    function __mul(self, other: Lou_Vec2 | number): Lou_Vec2
    function dot(self, other: Lou_Vec2): number
    function normalized(self): Lou_Vec2
    function squared_length(self): number
    function length(self): number
end

declare class Lou_Color
    r: number
    g: number
    b: number
    a: number
    function as_tuple(self): (number, number, number, number)
end
declare class Lou_Callback_Handle
    function destroy(self): ()
end
declare class Lou_Texture
    function size(self): (number, number)
    color: Lou_Color
    render: ((self: Lou_Texture, x: number, y: number, w: number?, h: number?) -> ())
        & ((self: Lou_Texture, dest: Lou_Rect, src: Lou_Rect?) -> ())
        & ((self: Lou_Texture, dst: Lou_Rect, angle: number, center: Lou_Vec2?, src: Lou_Rect?) -> ())
end
declare class Lou_Font
    font_size: number
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
    function draw_rect(self, rect: Lou_Rect): ()
    function fill_rect(self, rect: Lou_Rect): ()
    function draw_point(self, x: number, y: number): ()
    function draw_line(self, x1: number, y1: number, x2: number, y2: number): ()
    function set_draw_color(self, r: number, g: number, b: number, a: number?): ()
    function clear(self): ()
    render_texture: ((self: Lou_Renderer, texture: Lou_Texture, x: number, y: number) -> ())
        & ((self: Lou_Renderer, texture: Lou_Texture, dst: Lou_Rect, src: Lou_Rect?) -> ())
end

declare class Lou_Mouse
    function position(self): (number, number)
    function pressed(self, handler: (type: ('left'|'middle'|'right'), x: number, y: number)->()): Lou_Callback_Handle
    function released(self, handler: (type: ('left'|'middle'|'right'), x: number, y: number)->()): Lou_Callback_Handle
end
declare class Lou_Keyboard
    function pressed(self, handler: (key: string)->()): Lou_Callback_Handle
    function released(self, handler: (key: string)->()): Lou_Callback_Handle
    function is_pressed(self, key: string): boolean
end

declare class Lou_Create_Texture
    function from_text(self, font: Lou_Font, text: string, fg_color: Lou_Color): Lou_Texture
    function from_solid_color(self, solid_color: Lou_Color, w: number?, h: number?): Lou_Texture
    function draw(self, fn: (w: number, h: number)->(), w: number, h: number): Lou_Texture
    function load_image(self, file: string): Lou_Texture
end

declare class Lou_State 
    texture: Lou_Create_Texture
    renderer: Lou_Renderer
    window: Lou_Window
    mouse: Lou_Mouse
    keyboard: Lou_Keyboard
    console: Lou_Console
    function on_render(self, fn: ()->()): Lou_Callback_Handle
    function on_update(self, fn: (delta_seconds: number)->()): Lou_Callback_Handle
end

declare lou: Lou_State 
declare function Rect(x: number?, y: number?, w: number?, h: number?): Lou_Rect
declare function Vec2(x: number?, y: number?): Lou_Vec2
declare function Color(r: number?, g: number?, b: number?, a: number?): Lou_Color
declare function Font(file_path: string, font_size: number): Lou_Font
