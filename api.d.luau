declare print: <Tostring_Compatible_Args...>(Tostring_Compatible_Args...) -> ()
declare warn: <Tostring_Compatible_Args...>(Tostring_Compatible_Args...) -> ()
declare class Lou_Callback_Handle
    function destroy(self): ()
end
declare class disallow_t end
declare class vector_t extends vector
    X: number
    Y: number
    Z: number
    W: number
    x: number
    y: number
    z: number
    w: number
end
declare class vector3_t extends vector_t
end
declare class vector2_t extends vector3_t
end
declare class rect_t extends vector end
declare class line_t extends vector end
declare class color_t extends vector end
declare class Lou_Texture
    size: vector2_t
    color: color_t
    render: ((self: Lou_Texture, dst: rect_t, angle: number, src: rect_t?) -> ())
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
    function draw_rect(self, rect: rect_t): ()
    function fill_rect(self, rect: rect_t): ()
    function draw_point(self, x: number, y: number): ()
    function draw_line(self, line: line_t): ()
    function set_draw_color(self, draw_color: color_t): ()
    function clear(self): ()
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
    function from_text(self, font: Lou_Font, text: string, fg_color: vector3_t): Lou_Texture
    function from_solid_color(self, solid_color: vector3_t, w: number?, h: number?): Lou_Texture
    function draw(self, fn: (w: number, h: number)->(), w: number, h: number): Lou_Texture
    function load_image(self, file: string): Lou_Texture
end

declare function Font(file_path: string, font_size: number): Lou_Font
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
declare function rgba(r: number, g: number, b: number, a: number): color_t
declare function rgb(r: number, g: number, b: number): color_t
declare vec4: ((x: number, y: number, z: number, w: number) -> vector_t)
    & (() -> vector_t) & ((v: vector_t) -> vector_t)
declare vec3: ((x: number, y: number, z: number) -> vector3_t)
    & (() -> vector3_t) & ((v: vector) -> vector3_t)
declare vec2: ((x: number, y: number) -> vector2_t)
    & (() -> vector2_t) & ((v: vector) -> vector2_t)
declare rect: ((x: number, y: number, w: number, h: number) -> rect_t)
    & (() -> rect_t) & ((v: vector) -> rect_t)
declare line: ((x1: number, y1: number, x2: number, y2: number) -> line_t)
    & (() -> line_t) & ((v: vector) -> line_t)
declare function totuple(v: vector): (number, number, number, number)
declare function getx(v: vector, n: number?): number
