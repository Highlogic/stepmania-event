#ifndef NEW_FIELD_H
#define NEW_FIELD_H

#include <string>
#include <unordered_set>
#include <vector>

#include "ActorFrame.h"
#include "ArrowDefects.h"
#include "BitmapText.h"
#include "ModValue.h"
#include "NoteSkin.h"
#include "NoteData.h"
#include "Quad.h"
#include "Sprite.h"
#include "Song.h"

class NoteData;
class Steps;
class TimingData;
struct NoteField;

enum FieldLayerFadeType
{
	FLFT_Receptor,
	FLFT_Note,
	FLFT_Explosion,
	FLFT_None,
	NUM_FieldLayerFadeType,
	FieldLayerFadeType_Invalid
};
std::string const FieldLayerFadeTypeToString(FieldLayerFadeType fmt);
LuaDeclareType(FieldLayerFadeType);

enum FieldLayerTransformType
{
	FLTT_Full,
	FLTT_PosOnly,
	FLTT_None,
	NUM_FieldLayerTransformType,
	FieldLayerTransformType_Invalid
};
std::string const FieldLayerTransformTypeToString(FieldLayerTransformType fmt);
LuaDeclareType(FieldLayerTransformType);

struct FieldLayerRenderInfo
{
	FieldLayerFadeType fade_type;
	FieldLayerTransformType transform_type;
};

struct NoteFieldColumn : ActorFrame
{
	NoteFieldColumn();
	~NoteFieldColumn();

	struct render_note
	{
		render_note(NoteFieldColumn* column, NoteData::TrackMap::const_iterator
			column_begin, NoteData::TrackMap::const_iterator column_end,
			NoteData::TrackMap::const_iterator iter);
		double y_offset;
		double tail_y_offset;
		// tail_beat and tail_second are only used by lifts.
		double tail_beat;
		double tail_second;
		NoteData::TrackMap::const_iterator note_iter;
		mod_val_inputs input;
	};
	void add_children_from_layers(size_t column, std::vector<NoteSkinLayer>& layers);
	void set_note_data(size_t column, const NoteData* note_data,
		const TimingData* timing_data);
	void set_column_info(NoteField* field, size_t column, NoteSkinColumn* newskin,
		ArrowDefects* defects,
		NoteSkinData& skin_data, std::vector<Rage::Color>* player_colors,
		const NoteData* note_data, const TimingData* timing_data, double x);
	void take_over_mods(NoteFieldColumn& old_column);
	void set_defective_mode(bool mode);
	bool get_defective_mode();
	void set_speed(float time_spacing, float max_scroll_bpm,
		float scroll_speed, float scroll_bpm, float read_bpm, float music_rate);
	size_t get_mod_col() { return m_column+1; }

	void set_gameplay_zoom(double zoom);
	Rage::Color get_player_color(size_t pn);
	void get_hold_draw_time(TapNote const& tap, double const hold_beat,
		double& beat, double& second);
	void draw_hold(QuantizedHoldRenderData& data, render_note& note,
		double head_beat, double head_second,
		double tail_beat, double tail_second, bool is_lift);
	void set_displayed_time(double beat, double second);
	// update_displayed_time is called by the field when the Player draws in
	// gameplay.  set_displayed_beat is for lua to call when lua is controlling
	// the current beat.
	void update_displayed_time(double beat, double second);
	double get_curr_beat() { return m_curr_beat; }
	double get_curr_second() { return m_curr_second; }
	double get_beat_from_second(double second);
	double get_second_from_beat(double beat);
	void set_displayed_beat(double beat);
	void set_displayed_second(double second);
	bool timing_is_safe() { return m_timing_data != nullptr; }
	void set_pixels_visible_before(double pix)
	{
		m_pixels_visible_before_beat= pix;
	}
	void set_pixels_visible_after(double pix)
	{
		m_pixels_visible_after_beat= pix;
	}
	double get_upcoming_time()
	{
		return m_upcoming_time;
	}
	void set_upcoming_time(double up)
	{
		m_upcoming_time= up;
	}
	int y_offset_visible(double off)
	{
		if(off < first_y_offset_visible) { return -1; }
		if(off > last_y_offset_visible) { return 1; }
		return 0;
	}
	double calc_y_offset(mod_val_inputs& input);
	double calc_y_offset(double beat, double second);
	double head_y_offset()
	{
		return calc_y_offset(m_curr_beat, m_curr_second);
	}
	double get_reverse_shift()
	{
		return reverse_shift;
	}
	double quantization_for_time(mod_val_inputs& input)
	{
		double mult= m_quantization_multiplier.evaluate(input) * m_newskin->get_quantum_mult();
		double offset= m_quantization_offset.evaluate(input);
		return std::fmod((input.eval_beat * mult) + offset, 1.0);
	}
	void calc_transform(mod_val_inputs& input, Rage::transform& trans);
	void calc_transform_with_glow_alpha(mod_val_inputs& input,
		Rage::transform& trans);
	void calc_pos_only(mod_val_inputs& input, Rage::Vector3& out);
	void hold_render_transform(mod_val_inputs& input, Rage::transform& trans,
		bool do_rot);
	void calc_reverse_shift();
	double apply_reverse_shift(double y_offset);
	void apply_yoffset_to_pos(mod_val_inputs& input, Rage::Vector3& pos);
	void apply_column_mods_to_actor(Actor* act);
	void apply_note_mods_to_actor(Actor* act, double beat, double second,
		double y_offset, bool use_alpha, bool use_glow);
	float get_selection_glow();

	void build_render_lists();
	void draw_child(int child);
	Rage::transform const& get_head_trans() { return head_transform; }

	void pass_message_to_heads(Message& msg);
	void did_tap_note(TapNoteScore tns, bool bright);
	void did_hold_note(HoldNoteScore hns, bool bright);
	void set_hold_status(TapNote const* tap, bool start, bool end);
	void set_pressed(bool on);

	virtual void UpdateInternal(float delta);
	virtual bool EarlyAbortDraw() const;
	void imitate_did_note(TapNote const& tap);
	void update_upcoming(double beat, double second);
	void update_active_hold(TapNote const& tap);
	virtual void DrawPrimitives();
	void position_actor_at_column_head(Actor* act, FieldLayerRenderInfo& info);

	virtual void AddChild(Actor* act);
	virtual void RemoveChild(Actor* act);
	virtual void ChildChangedDrawOrder(Actor* child);

	virtual void PushSelf(lua_State *L);
	virtual NoteFieldColumn* Copy() const;

	NotePlayerizeMode get_playerize_mode() { return m_playerize_mode; }
	void set_playerize_mode(NotePlayerizeMode mode);

	void set_layer_fade_type(Actor* child, FieldLayerFadeType type);
	FieldLayerFadeType get_layer_fade_type(Actor* child);
	void set_layer_transform_type(Actor* child, FieldLayerTransformType type);
	FieldLayerTransformType get_layer_transform_type(Actor* child);

	bool m_use_game_music_beat;
	bool m_show_unjudgable_notes;
	bool m_speed_segments_enabled;
	bool m_scroll_segments_enabled;
	bool m_holds_skewed_by_mods;
	bool m_twirl_holds;
	bool m_use_moddable_hold_normal;

	struct column_status
	{
		column_status()
			:active_hold(nullptr), prev_active_hold(nullptr)
		{}
		double upcoming_beat_dist;
		double upcoming_second_dist;
		double anim_percent;
		TapNote const* active_hold;
		TapNote const* prev_active_hold;
		bool found_upcoming;
		bool in_reverse;
	};
	column_status m_status;

	ModManager m_mod_manager;
	// If you add another ModifiableValue member, be sure to add it to the
	// loop in set_note_data.  They need to have the timing data passed to
	// them so mods can have start and end times.
	// Also add new ModifiableValue members to the take_over_mods function.
	ModifiableValue m_time_offset;
	ModifiableValue m_quantization_multiplier;
	ModifiableValue m_quantization_offset;

	ModifiableValue m_speed_mod;
	ModifiableValue m_lift_pretrail_length;

	ModifiableVector3 m_y_offset_vec_mod;

	ModifiableValue m_reverse_offset_pixels;
	ModifiableValue m_reverse_scale;
	ModifiableValue m_center_percent;

	ModifiableTransform m_note_mod;
	ModifiableTransform m_column_mod;
	ModifiableVector3 m_hold_normal_mod;

	ModifiableValue m_note_alpha;
	ModifiableValue m_note_glow;
	ModifiableValue m_receptor_alpha;
	ModifiableValue m_receptor_glow;
	ModifiableValue m_explosion_alpha;
	ModifiableValue m_explosion_glow;

	TimingSource m_timing_source;

	double m_selection_start;
	double m_selection_end;

private:
	void did_tap_note_internal(TapNoteScore tns, bool bright);
	void did_hold_note_internal(HoldNoteScore hns, bool bright);
	void draw_child_internal();
	void add_renderable_to_lists(render_note& renderable);

	double m_curr_beat;
	double m_curr_displayed_beat;
	double m_curr_second;
	double m_prev_curr_second;
	double m_pixels_visible_before_beat;
	double m_pixels_visible_after_beat;
	double m_upcoming_time;
	size_t m_column;
	NotePlayerizeMode m_playerize_mode;
	NoteSkinColumn* m_newskin;
	std::vector<Rage::Color>* m_player_colors;
	NoteField* m_field;
	std::vector<FieldLayerRenderInfo> m_layer_render_info;

	ArrowDefects* m_defective_mods;
	bool m_in_defective_mode;

	const NoteData* m_note_data;
	const TimingData* m_timing_data;

	double m_gameplay_zoom;

	Quad m_area_highlight;

	// Data that needs to be stored for rendering below here.
	// Holds and taps are put into different lists because they have to be
	// rendered in different phases.  All hold bodies must be drawn first, then
	// all taps, so the taps appear on top of the hold bodies and are not
	// obscured.
	void draw_holds_internal();
	void draw_lifts_internal();
	void draw_taps_internal();
	void draw_selection_internal();
	int note_row_closest_to_current_time;
	std::list<render_note> render_holds;
	std::list<render_note> render_lifts;
	std::list<render_note> render_taps;
	int curr_render_child;
	// Calculating the effects of reverse and center for every note is costly.
	// Only do it once per frame and store the result.
	double reverse_shift;
	double reverse_scale;
	double reverse_scale_sign;
	double first_y_offset_visible;
	double last_y_offset_visible;
	Rage::transform head_transform;
	double receptor_alpha;
	double receptor_glow;
	double explosion_alpha;
	double explosion_glow;
	bool pressed;
	bool was_pressed;
};

enum FieldVanishType
{
	FVT_RelativeToParent,
	FVT_RelativeToSelf,
	FVT_RelativeToOrigin,
	NUM_FieldVanishType,
	FieldVanishType_Invalid
};
std::string const FieldVanishTypeToString(FieldVanishType fmt);
LuaDeclareType(FieldVanishType);

struct NoteField : ActorFrame
{
	NoteField();
	~NoteField();
	virtual void UpdateInternal(float delta);
	virtual bool EarlyAbortDraw() const;
	virtual void PreDraw();
	virtual void PostDraw();
	void draw_board();
	void draw_up_to_draw_order(int order);
	virtual void DrawPrimitives();
	void position_actor_at_column_head(Actor* act, FieldLayerRenderInfo& info,
		size_t col);
	double get_receptor_y();

	virtual void PushSelf(lua_State *L);
	virtual NoteField* Copy() const;

	virtual void HandleMessage(Message const& msg);

	virtual void AddChild(Actor* act);
	virtual void RemoveChild(Actor* act);
	virtual void ChildChangedDrawOrder(Actor* child);

	void push_columns_to_lua(lua_State* L);
	double get_field_width() { return m_field_width; }
	size_t get_num_columns() { return m_columns.size(); }

	void set_player_color(size_t pn, Rage::Color const& color);
	void set_gameplay_zoom(double zoom);

	void clear_steps();
	void set_skin(std::string const& skin_name, LuaReference& skin_params);
	std::string const& get_skin();
	void set_steps(Steps* data);
	void set_note_data(NoteData* note_data, TimingData* timing, StepsType stype);
	// set_player_number exists only so that the notefield layers can have
	// per-player configuration on gameplay.  Using it for any other purpose
	// is forbidden.
	void set_player_number(PlayerNumber pn);
	// set_player_options is for supporting the old ArrowEffects mods in
	// defective mode.
	void set_player_options(PlayerOptions* options);
	void set_read_bpm(float read_bpm) {m_defective_mods.set_read_bpm(read_bpm);}
	void set_defective_mode(bool mode);
	bool get_defective_mode();
	void disable_defective_mode();
	void set_speed(float time_spacing, float max_scroll_bpm,
		float scroll_speed, float scroll_bpm, float read_bpm, float music_rate);
	void disable_speed_scroll_segments();

	void turn_on_edit_mode();
	double get_selection_start();
	double get_selection_end();
	void set_selection_start(double value);
	void set_selection_end(double value);

	void set_layer_fade_type(Actor* child, FieldLayerFadeType type);
	FieldLayerFadeType get_layer_fade_type(Actor* child);

	void update_displayed_time(double beat, double second);
	double get_curr_beat() { return m_curr_beat; }
	double get_curr_second() { return m_curr_second; }
	double get_beat_from_second(double second);
	double get_second_from_beat(double beat);
	void set_displayed_beat(double beat);
	void set_displayed_second(double second);
	bool timing_is_safe() { return m_timing_data != nullptr; }

	void did_tap_note(size_t column, TapNoteScore tns, bool bright);
	void did_hold_note(size_t column, HoldNoteScore hns, bool bright);
	void set_pressed(size_t column, bool on);

	ModManager m_mod_manager;
	ModifiableTransform m_trans_mod;
	ModifiableValue m_receptor_alpha;
	ModifiableValue m_receptor_glow;
	ModifiableValue m_explosion_alpha;
	ModifiableValue m_explosion_glow;
	ModifiableValue m_fov_mod;
	ModifiableValue m_vanish_x_mod;
	ModifiableValue m_vanish_y_mod;
	// To allow the Player actor the field is inside to be moved around without
	// causing skew problems, the field adds the vanish position to the actor
	// position.  m_vanish_type tells the field whether to look at its parent,
	// itself, or nothing when offsetting the vanish point.
	// The relative to self and relative to nothing choices exist for when a
	// field is displayed without a player actor.
	FieldVanishType m_vanish_type;
	bool m_being_drawn_by_player;
	bool m_draw_beat_bars;
	bool m_in_edit_mode;
	// OITG bug:  Actor::SetZoom only sets X and Y.  When mini is applied to
	// the notefield with SetZoom, it does not affect the range of bumpy.
	// m_oitg_zoom_mode provides compatibility with that bug.  Only used in
	// defective mode.
	bool m_oitg_zoom_mode;

	// Only draw the text from one bg change layer. -Kyz
	BackgroundLayer m_visible_bg_change_layer;

	struct field_draw_entry
	{
		int column;
		int child;
		int draw_order;
	};
	void add_draw_entry(int column, int child, int draw_order);
	void remove_draw_entry(int column, int child);
	void change_draw_entry(int column, int child, int new_draw_order);
	void clear_column_draw_entries();
	double update_z_bias();

	// selection_glow is needed for making notes in the selected area glow.
	float selection_glow;

private:
	void draw_entry(field_draw_entry& entry);
	void draw_beat_bar(mod_val_inputs& input, int state, float alpha);
	void draw_field_text(mod_val_inputs& input,
		double x_offset, float side_sign, float horiz_align,
		Rage::Color const& color, Rage::Color const& glow);
	void draw_beat_bars_internal();
	bool draw_beat_bars_step(float const start_beat, float const step,
		Rage::Color const& measure_number_color, Rage::Color const&
		measure_number_glow, bool needs_second);
	void draw_bg_change_list(bool needs_second,
		float const first_beat, float const last_beat,
		vector<BackgroundChange>& changes, Rage::Color const& color,
		Rage::Color const& text_glow);
	void reload_columns(NoteSkinLoader const* new_loader, LuaReference& new_params);
	double m_curr_beat;
	double m_curr_second;
	double m_field_width;

	// The player number has to be stored so that it can be passed to the
	// column layers when they're loaded.
	PlayerNumber m_pn;
	ArrowDefects m_defective_mods;
	bool m_in_defective_mode;

	bool m_own_note_data;
	NoteData* m_note_data;
	const TimingData* m_timing_data;
	StepsType m_steps_type;
	std::vector<NoteFieldColumn> m_columns;
	NoteSkinData m_newskin;
	NoteSkinLoader m_skin_walker;
	LuaReference m_skin_parameters;
	std::vector<Rage::Color> m_player_colors;
	std::vector<FieldLayerRenderInfo> m_layer_render_info;

	Sprite m_beat_bars;
	BitmapText m_field_text;

	vector<field_draw_entry> m_draw_entries;
	size_t m_first_undrawn_entry;
	int m_curr_draw_limit;
	double m_gameplay_zoom;

	// Evaluation results of the mods need to be stored because Draw is called
	// twice: once for the board, once for everything else.  So the mods can't
	// be evaluated in Draw.  They are evaluated in update_displayed_time. -Kyz
	double evaluated_receptor_alpha;
	double evaluated_receptor_glow;
	double evaluated_explosion_alpha;
	double evaluated_explosion_glow;
	// The old distant and hallway mods would shift the notefield before
	// rendering, then move it back.  defective_render_y is used to store the
	// calculated shifted y.
	double defective_render_y;
	double original_y;

	double curr_z_bias;
};

#endif
<<<<<<< e75f5969bddc861e6476040479d3e7947a44dddb
=======

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE. 
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */
>>>>>>> Initial GPL commit for the event fork
