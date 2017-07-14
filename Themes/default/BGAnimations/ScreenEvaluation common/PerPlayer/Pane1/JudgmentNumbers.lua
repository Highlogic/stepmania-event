local player = ...
-- Use enum's reverse lookup to set pn to 1 if PLAYER_1, or 2 if PLAYER_2
local pn = PlayerNumber:Reverse()[player] + 1

local stats = STATSMAN:GetCurStageStats():GetPlayerStageStats(player)

local TapNoteScores = {
	Types = { 'W1', 'W2', 'W3', 'W4', 'W5', 'Miss' },
	-- x values for P1 and P2
	x = { 94, 94, 94, 94 ,94 ,94 }
}

local RadarCategories = {
	Types = { 'Holds', 'Mines'},
	-- x values for P1 and P2
	x = { 218, 218, 218, 218, 218, 218  }
}

local StomperZColors = {
	color("#FFFFFF"),	-- white
	color("#e29c18"),	-- gold
	color("#66c955"),	-- green
	color("#21CCE8"),	-- blue
	color("#000000"),	-- black
	color("#ff0000")	-- red
}

local ECFAColors = {
	color("#21CCE8"),	-- blue
	color("#FFFFFF"),	-- white
	color("#e29c18"),	-- gold
	color("#66c955"),	-- green
	color("#9e00f7"),	-- purple
	color("#ff0000")	-- red
}


local t = Def.ActorFrame{
	InitCommand=cmd(zoom, 0.8; xy,-90,_screen.cy-24),
	OnCommand=function(self)
	end
}

-- do "regular" TapNotes first
for index, window in ipairs(TapNoteScores.Types) do

	local number = stats:GetTapNoteScores( "TapNoteScore_"..window )

	-- actual numbers
	t[#t+1] = Def.RollingNumbers{
		Font="_ScreenEvaluation numbers",
		InitCommand=function(self)
			self:zoom(0.4):horizalign(right)

			-- check for Decents/Way Offs
			local gmods = SL.Global.ActiveModifiers

			-- If Way Offs were turned off, the leading 0s should not
			-- be colored any differently than the (lack of) JudgmentNumber,
			-- so load a unique Metric group.
			if gmods.DecentsWayOffs == "Decents Only" and window == "W5" then
				self:Load("RollingNumbersEvaluationNoDecentsWayOffs")
				self:diffuse(color("#444444"))

			-- If both Decents and WayOffs were turned off, the same logic applies.
			elseif gmods.DecentsWayOffs == "Off" and (window == "W4" or window == "W5") then
				self:Load("RollingNumbersEvaluationNoDecentsWayOffs")
				self:diffuse(color("#444444"))

			-- Otherwise, We want leading 0s to be dimmed, so load the Metrics
			-- group "RollingNumberEvaluationA"	which does that for us.
			else
				self:Load("RollingNumbersEvaluationA")
			end
		end,
		
		BeginCommand=function(self)
			self:y((player == PLAYER_1 and 60) or 60 )
			self:x((player == PLAYER_1 and (index-1)*100 -95) or (index-1)*100 -95)
			self:targetnumber(number)
		end
	}

end

-- then handle holds, mines, hands, rolls
for index, RCType in ipairs(RadarCategories.Types) do

	local performance = stats:GetRadarActual():GetValue( "RadarCategory_"..RCType )
	local possible = stats:GetRadarPossible():GetValue( "RadarCategory_"..RCType )

	-- player performace value
	t[#t+1] = Def.RollingNumbers{
		Font="_ScreenEvaluation numbers",
		InitCommand=cmd(zoom,0.3; horizalign, right; Load, "RollingNumbersEvaluationB"),
		BeginCommand=function(self)
			self:x((index-1)*100 + 480)
			self:y((player == PLAYER_1 and 60) or 60)
			self:targetnumber(performance)
		end
	}

	--  slash
	t[#t+1] = LoadFont("_miso")..{
		Text="/",
		InitCommand=cmd(diffuse,color("#5A6166"); zoom, 1.25; horizalign, right),
		BeginCommand=function(self)
			self:x((index-1)*100 + 490)
			self:y((player == PLAYER_1 and 60) or 60)
		end
	}

	-- possible value
	t[#t+1] = LoadFont("_ScreenEvaluation numbers")..{
		InitCommand=cmd(zoom,0.3; horizalign, right),
		BeginCommand=function(self)
			self:x((index-1)*100 + 520)
			self:y((player == PLAYER_1 and 60) or 60)
			self:settext(("%03.0f"):format(possible))
			local leadingZeroAttr = { Length=3-tonumber(tostring(possible):len()); Diffuse=color("#5A6166") }
			self:AddAttribute(0, leadingZeroAttr )
		end
	}
end

return t