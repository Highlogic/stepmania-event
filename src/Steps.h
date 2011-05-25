#ifndef STEPS_H
#define STEPS_H

#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"
#include "Grade.h"
#include "RadarValues.h"
#include "Difficulty.h"
#include "RageUtil_AutoPtr.h"
#include "RageUtil_CachedObject.h"
#include "TimingData.h"

class Profile;
class NoteData;
struct lua_State;

/** 
 * @brief Allow only 12 characters for the length of edits.
 *
 * This is a holdover from In The Groove: this may perhaps be not needed in the future.
 */
const int MAX_EDIT_STEPS_DESCRIPTION_LENGTH = 12;

/** 
 * @brief Holds note information for a Song.
 *
 * A Song may have one or more Notes. */
class Steps
{
public:
	/** @brief Set up the Steps with initial values. */
	Steps();
	/** @brief Destroy the Steps that are no longer needed. */
	~Steps();

	// initializers
	void AutogenFrom( const Steps *parent, StepsType ntTo );
	void CopyFrom( Steps* pSource, StepsType ntTo, float fMusicLengthSeconds );
	void CreateBlank( StepsType ntTo );

	void Compress() const;
	void Decompress() const;
	/** 
	 * @brief Determine if these steps were created by the autogenerator.
	 * @return true if they were, false otherwise.
	 */
	bool IsAutogen() const				{ return parent != NULL; }

	/**
	 * @brief Determine if this set of Steps is an edit.
	 *
	 * Edits have a special value of difficulty to make it easy to determine.
	 * @return true if this is an edit, false otherwise.
	 */
	bool IsAnEdit() const				{ return m_Difficulty == Difficulty_Edit; }
	/**
	 * @brief Determine if this set of Steps is a player edit.
	 *
	 * Player edits also have to be loaded from a player's profile slot, not the machine.
	 * @return true if this is a player edit, false otherwise. */
	bool IsAPlayerEdit() const			{ return IsAnEdit() && GetLoadedFromProfileSlot() < ProfileSlot_Machine; }
	/**
	 * @brief Determine if these steps were loaded from a player's profile.
	 * @return true if they were from a player profile, false otherwise.
	 */
	bool WasLoadedFromProfile() const		{ return m_LoadedFromProfile != ProfileSlot_Invalid; }
	ProfileSlot GetLoadedFromProfileSlot() const	{ return m_LoadedFromProfile; }
	/**
	 * @brief Retrieve the description used for this edit.
	 * @return the description used for this edit.
	 */
	RString GetDescription() const			{ return Real()->m_sDescription; }
	/**
	 * @brief Retrieve the ChartStyle used for this chart.
	 * @return the description used for this chart.
	 */
	RString GetChartStyle() const			{ return Real()->m_sChartStyle; }
	/**
	 * @brief Retrieve the difficulty used for this edit.
	 * @return the difficulty used for this edit.
	 */
	Difficulty GetDifficulty() const		{ return Real()->m_Difficulty; }
	/**
	 * @brief Retrieve the meter used for this edit.
	 * @return the meter used for this edit.
	 */
	int GetMeter() const				{ return Real()->m_iMeter; }
	const RadarValues& GetRadarValues( PlayerNumber pn ) const { return Real()->m_CachedRadarValues[pn]; }
	/**
	 * @brief Retrieve the author credit used for this edit.
	 * @return the author credit used for this edit.
	 */
	RString GetCredit() const			{ return Real()->m_sCredit; }

	void SetFilename( RString fn )			{ m_sFilename = fn; }
	RString GetFilename() const			{ return m_sFilename; }
	void SetSavedToDisk( bool b )			{ DeAutogen(); m_bSavedToDisk = b; }
	bool GetSavedToDisk() const			{ return Real()->m_bSavedToDisk; }
	void SetDifficulty( Difficulty dc )		{ SetDifficultyAndDescription( dc, GetDescription() ); }
	void SetDescription( RString sDescription ) 	{ SetDifficultyAndDescription( this->GetDifficulty(), sDescription ); }
	void SetDifficultyAndDescription( Difficulty dc, RString sDescription );
	void SetCredit( RString sCredit );
	void SetChartStyle( RString sChartStyle );
	static bool MakeValidEditDescription( RString &sPreferredDescription );	// return true if was modified

	void SetLoadedFromProfile( ProfileSlot slot )	{ m_LoadedFromProfile = slot; }
	void SetMeter( int meter );
	void SetCachedRadarValues( const RadarValues v[NUM_PLAYERS] );
	float PredictMeter() const;

	unsigned GetHash() const;
	void GetNoteData( NoteData& noteDataOut ) const;
	NoteData GetNoteData() const;
	void SetNoteData( const NoteData& noteDataNew );
	void SetSMNoteData( const RString &notes_comp );
	void GetSMNoteData( RString &notes_comp_out ) const;

	void TidyUpData();
	void CalculateRadarValues( float fMusicLengthSeconds );

	/** @brief Timing data */
	TimingData m_Timing;
	
	/**
	 * @brief Determine if the Steps have any major timing changes during gameplay.
	 * @return true if it does, or false otherwise. */
	bool HasSignificantTimingChanges() const;

	// Lua
	void PushSelf( lua_State *L );

	StepsType			m_StepsType;

	CachedObject<Steps> m_CachedObject;
	
	/**
	 * @brief Determine if the Steps use Split Timing by comparing the Song it's in.
	 * @return true if the Step and Song use different timings, false otherwise. */
	bool UsesSplitTiming() const;

private:
	inline const Steps *Real() const		{ return parent ? parent : this; }
	void DeAutogen( bool bCopyNoteData = true ); /* If this Steps is autogenerated, make it a real Steps. */

	/**
	 * @brief Identify this Steps' parent.
	 *
	 * If this Steps is autogenerated, this will point to the autogen
	 * source.  If this is true, m_sNoteDataCompressed will always be empty. */
	const Steps			*parent;

	/* We can have one or both of these; if we have both, they're always identical.
	 * Call Compress() to force us to only have m_sNoteDataCompressed; otherwise, creation of 
	 * these is transparent. */
	mutable HiddenPtr<NoteData>	m_pNoteData;
	mutable bool			m_bNoteDataIsFilled;
	mutable RString			m_sNoteDataCompressed;

	/** @brief The name of the file where these steps are stored. */
	RString				m_sFilename;
	/** @brief true if these Steps were loaded from or saved to disk. */
	bool				m_bSavedToDisk;	
	/** @brief What profile was used? This is ProfileSlot_Invalid if not from a profile. */
	ProfileSlot			m_LoadedFromProfile;

	/* These values are pulled from the autogen source first, if there is one. */
	/** @brief The hash of the steps. This is used only for Edit Steps. */
	mutable unsigned		m_iHash;
	/** @brief The name of the edit, or some other useful description.
	 This used to also contain the step author's name. */
	RString				m_sDescription;
	/** @brief The style of the chart. (e.g. "Pad", "Keyboard") */
	RString				m_sChartStyle;
	/** @brief The difficulty that these steps are assigned to. */
	Difficulty			m_Difficulty;
	/** @brief The numeric difficulty of the Steps, ranging from MIN_METER to MAX_METER. */
	int				m_iMeter;
	/** @brief The radar values used for each player. */
	RadarValues			m_CachedRadarValues[NUM_PLAYERS];
	/** @brief The name of the person who created the Steps. */
	RString				m_sCredit;
};

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
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
 */
