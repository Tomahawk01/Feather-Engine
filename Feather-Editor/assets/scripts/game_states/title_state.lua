TitleState = {}
TitleState.__index = TitleState

function TitleState:Create(stack)
	local this =
	{
		m_Stack = stack,
		m_Title = Entity("title", ""),
		m_PressEnter = Entity("press_enter", "")
	}

	this.m_Title:add_component(Transform(vec2(100, 100), vec2(1, 1), 0))
	this.m_Title:add_component(TextComponent("testFont", "Title State"))

	this.m_PressEnter:add_component(Transform(vec2(100, 200), vec2(1, 1), 0))
	this.m_PressEnter:add_component(TextComponent("testFont", "PRESS ENTER!"))

	local state = State("title state")
	state:set_variable_table(this)
	state:set_on_enter(
		function()
			print("Enter Title State")
		end
	)
	state:set_on_exit(
		function()
			this:OnExit()
		end
	)

	state:set_on_update(
		function(dt)
		end
	)

	state:set_on_render(
		function()
		end
	)

	state:set_handle_inputs(
		function()
			this:HandleInputs()
		end
	)

	setmetatable(this, self)
	return state
end

function TitleState:OnExit()
	self.m_Title:kill()
	self.m_PressEnter:kill()
end

function TitleState:HandleInputs()
	if (Keyboard.just_released(KEY_ENTER)) then
		self.m_Stack:change_state(GameState:Create(self.m_Stack))
		return
	end
end