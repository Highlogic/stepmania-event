return Def.ActorFrame{

	-- Song Completion Meter
	Def.ActorFrame{
		Name="SongMeter",
		InitCommand=cmd(xy, _screen.cx, 11 ),

		Def.SongMeterDisplay{
			StreamWidth=_screen.w-45,
			Stream=Def.Quad{ 
				InitCommand=cmd(zoomy,18; diffuse,DifficultyIndexColor(5))
			}
		},

		Border( _screen.w-45, 22, 2 ),
	},

	-- Song Title
	LoadFont("_miso")..{
		Name="SongTitle",
		InitCommand=cmd(zoom,0.8; shadowlength,0.6; maxwidth, _screen.w/2.5 - 10; xy, _screen.cx, 11 ),
		CurrentSongChangedMessageCommand=cmd(playcommand, "Update"),
		UpdateCommand=function(self)
			local song = GAMESTATE:GetCurrentSong()
			self:settext( song and song:GetDisplayFullTitle() or "" )
		end
	}
}