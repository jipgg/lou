export type Renderable = {
    render: (self: any) -> (),
}
export type Updatable = {
    update: (self: any, delta_seconds: number) -> (),
}
local RESOURCE_DIR: "resources" = "resources"
local M = {
    main_font = Font(`{RESOURCE_DIR}/main.ttf`, 100),
    _renderables = {} :: {Renderable},
    _updatables = {} :: {Updatable},
}

function M.add(object: Renderable & Updatable)
    table.insert(M._updatables, object)
    table.insert(M._renderables, object)
end

local function find_if<T>(t: {T}, predicate: (e: T) -> boolean): number?
    for i, v in t do
        if predicate(v) then
            return i
        end
    end
    return
end
local function remove_if<T>(t: {T}, predicate: (T) -> boolean): boolean
    local found = find_if(t, predicate)
    if found then
        table.remove(t, found)
        return true
    else
        return false
    end
end

function M.remove(object: any)
    local pred = function(e) return rawequal(object, e) end
    remove_if(M._updatables, pred)
    remove_if(M._renderables, pred)
end

function M.update(delta_seconds: number)
    for _, object in M._updatables do
        object:update(delta_seconds)
    end
end

function M.render()
    for _, object in M._renderables do
        object:render()
    end
end

return M

