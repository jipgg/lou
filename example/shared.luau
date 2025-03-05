export type Renderable = {
    render: (self: any) -> (),
}
export type Updatable = {
    update: (self: any, delta_seconds: number) -> (),
}
local RESOURCE_DIR: "resources" = "resources"
local M = {
    RESOURCE_DIR = RESOURCE_DIR,
    main_font = Font(`{RESOURCE_DIR}/main.ttf`, 100),
    renderables = {} :: {Renderable},
    updatables = {} :: {Updatable},
}

return M
