local component = require "engine.component"
local M = {}

function M:start()

end

function M:update()
	component.rotate(self, 0, self.speed, 0)
end

return M

