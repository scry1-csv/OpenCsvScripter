#pragma once

#include "nlohmann/json.hpp"
#include "FunscriptAction.h"
#include "OFS_Reflection.h"
#include "OFS_Serialization.h"
#include "OFS_BinarySerialization.h"

#include <string>
#include <memory>
#include <chrono>

#include "OFS_Util.h"
#include "SDL_mutex.h"

#include "FunscriptSpline.h"
#include "OFS_Profiling.h"

#include "EASTL/sort.h"

class FunscriptUndoSystem;

class FunscriptEvents
{
public:
	static int32_t FunscriptActionsChangedEvent;
	static int32_t FunscriptSelectionChangedEvent;

	static void RegisterEvents() noexcept;
};

class Funscript
{
public:
	struct FunscriptData {
		FunscriptArray Actions;
		FunscriptArray selection;
	};

	struct Metadata {
		std::string type = "basic";
		std::string title;
		std::string creator;
		std::string script_url;
		std::string video_url;
		std::vector<std::string> tags;
		std::vector<std::string> performers;
		std::string description;
		std::string license;
		std::string notes;
		int64_t duration = 0;

		template <class Archive>
		inline void reflect(Archive& ar) {
			OFS_REFLECT(type, ar);
			OFS_REFLECT(title, ar);
			OFS_REFLECT(creator, ar);
			OFS_REFLECT(script_url, ar);
			OFS_REFLECT(video_url, ar);
			OFS_REFLECT(tags, ar);
			OFS_REFLECT(performers, ar);
			OFS_REFLECT(license, ar);
			OFS_REFLECT(duration, ar);
			OFS_REFLECT(description, ar);
			OFS_REFLECT(notes, ar);
		}

		bool loadFromFunscript(const std::string& path) noexcept;
		bool writeToFunscript(const std::string& path) noexcept;

		template<typename S>
		void serialize(S& s)
		{
			s.ext(*this, bitsery::ext::Growable{},
				[](S& s, Metadata& o) {
					s.container(o.tags, o.tags.max_size(), [](S& s, std::string& tag) {
						s.text1b(tag, tag.max_size());
						});
					s.container(o.performers, o.performers.max_size(), [](S& s, std::string& performer) {
						s.text1b(performer, performer.max_size());
						});
					s.text1b(o.type, o.type.max_size());
					s.text1b(o.title, o.title.max_size());
					s.text1b(o.creator, o.creator.max_size());
					s.text1b(o.script_url, o.script_url.max_size());
					s.text1b(o.video_url, o.video_url.max_size());
					s.text1b(o.description, o.description.max_size());
					s.text1b(o.license, o.license.max_size());
					s.text1b(o.notes, o.notes.max_size());
					s.value8b(o.duration);
				});
		}
	};
	// this is used when loading from json or serializing to json
	Funscript::Metadata LocalMetadata;

	template<typename S>
	void serialize(S& s)
	{
		s.ext(*this, bitsery::ext::Growable{},
			[](S& s, Funscript& o) {
				s.container(o.data.Actions, o.data.Actions.kMaxSize);
				// Metadata is centralized in OFS_Project
				s.text1b(o.CurrentPath, o.CurrentPath.max_size());
				s.text1b(o.Title, o.Title.max_size());
			});
	}

private:
	nlohmann::json Json;
	std::chrono::system_clock::time_point editTime;
	bool scriptOpened = false;
	bool funscriptChanged = false; // used to fire only one event every frame a change occurs
	bool unsavedEdits = false; // used to track if the script has unsaved changes
	bool selectionChanged = false;
	SDL_mutex* saveMutex = nullptr;
	FunscriptData data;

	void checkForInvalidatedActions() noexcept;

	inline FunscriptAction* getAction(FunscriptAction action) noexcept
	{
		OFS_PROFILE(__FUNCTION__);
		if (data.Actions.empty()) return nullptr;
		auto it = data.Actions.find(action);
		return it != data.Actions.end() ? it : nullptr;
	}

	inline FunscriptAction* getActionAtTime(FunscriptArray& actions, int32_t time_ms, uint32_t max_error_ms) noexcept
	{
		OFS_PROFILE(__FUNCTION__);
		if (actions.empty()) return nullptr;
		// gets an action at a time with a margin of error
		int32_t smallestError = std::numeric_limits<int32_t>::max();
		FunscriptAction* smallestErrorAction = nullptr;

		int i = 0;
		auto it = data.Actions.lower_bound(FunscriptAction(time_ms-max_error_ms, 0));
		if (it != data.Actions.end()) {
			i = std::distance(data.Actions.begin(), it);
			if (i > 0) --i;
		}

		for (; i < actions.size(); i++) {
			auto& action = actions[i];

			if (action.at > (time_ms + (max_error_ms / 2)))
				break;

			int32_t error = std::abs(time_ms - action.at);
			if (error <= max_error_ms) {
				if (error <= smallestError) {
					smallestError = error;
					smallestErrorAction = &action;
				}
				else {
					break;
				}
			}
		}
		return smallestErrorAction;
	}

	inline FunscriptAction* getNextActionAhead(int32_t time_ms) noexcept
	{
		OFS_PROFILE(__FUNCTION__);
		if (data.Actions.empty()) return nullptr;
		auto it = data.Actions.upper_bound(FunscriptAction(time_ms, 0));
		return it != data.Actions.end() ? it : nullptr;
	}

	inline FunscriptAction* getPreviousActionBehind(int32_t time_ms) noexcept
	{
		OFS_PROFILE(__FUNCTION__);
		if (data.Actions.empty()) return nullptr;
		auto it = data.Actions.lower_bound(FunscriptAction(time_ms, 0));
		return it-1 >= data.Actions.begin() ? it - 1 : nullptr;
	}

	void moveActionsTime(std::vector<FunscriptAction*> moving, int32_t time_offset);
	void moveActionsPosition(std::vector<FunscriptAction*> moving, int32_t pos_offset);
	inline void sortSelection() noexcept { sortActions(data.selection); }
	inline void sortActions(FunscriptArray& actions) noexcept {
		OFS_PROFILE(__FUNCTION__);
		eastl::sort(actions.begin(), actions.end());
	}
	inline void addAction(FunscriptArray& actions, FunscriptAction newAction) noexcept {
		OFS_PROFILE(__FUNCTION__);
		actions.emplace(newAction);
		NotifyActionsChanged(true);
	}

	inline void NotifySelectionChanged() noexcept {
		selectionChanged = true;
	}

	void loadMetadata() noexcept;
	void saveMetadata() noexcept;

	void startSaveThread(const std::string& path, FunscriptArray&& actions, nlohmann::json&& json) noexcept;	
	std::string CurrentPath;
public:
	Funscript();
	~Funscript();

	inline void NotifyActionsChanged(bool isEdit) noexcept {
		funscriptChanged = true;
		if (isEdit && !unsavedEdits) {
			unsavedEdits = true;
			editTime = std::chrono::system_clock::now();
		}
	}

	std::unique_ptr<FunscriptUndoSystem> undoSystem;

	std::string Title;
	bool Enabled = true;

	inline void UpdatePath(const std::string& path) noexcept {
		CurrentPath = path;
		Title = Util::PathFromString(CurrentPath)
			.replace_extension("")
			.filename()
			.u8string();
	}

	inline void SetSavedFromOutside() noexcept { unsavedEdits = false;	}

	inline const std::string& Path() const noexcept { return CurrentPath; }

	inline void rollback(FunscriptData&& data) noexcept { this->data = std::move(data); NotifyActionsChanged(true); }
	inline void rollback(const FunscriptData& data) noexcept { this->data = data; NotifyActionsChanged(true); }
	void update() noexcept;

	bool open(const std::string& file);
	void save() noexcept { save(CurrentPath, true); }
	void save(const std::string& path, bool override_location = true);
	
	const FunscriptData& Data() const noexcept { return data; }
	const auto& Selection() const noexcept { return data.selection; }
	const auto& Actions() const noexcept { return data.Actions; }

	inline const FunscriptAction* GetAction(FunscriptAction action) noexcept { return getAction(action); }
	inline const FunscriptAction* GetActionAtTime(int32_t time_ms, uint32_t error_ms) noexcept { return getActionAtTime(data.Actions, time_ms, error_ms); }
	inline const FunscriptAction* GetNextActionAhead(int32_t time_ms) noexcept { return getNextActionAhead(time_ms); }
	inline const FunscriptAction* GetPreviousActionBehind(int32_t time_ms) noexcept { return getPreviousActionBehind(time_ms); }
	inline const FunscriptAction* GetClosestAction(int32_t time_ms) noexcept { return getActionAtTime(data.Actions, time_ms, std::numeric_limits<uint32_t>::max()); }

	float GetPositionAtTime(int32_t time_ms) noexcept;
	
	inline void AddAction(FunscriptAction newAction) noexcept { addAction(data.Actions, newAction); }
	void AddActionRange(const FunscriptArray& range, bool checkDuplicates = true) noexcept;

	bool EditAction(FunscriptAction oldAction, FunscriptAction newAction) noexcept;
	void AddEditAction(FunscriptAction action, float frameTimeMs) noexcept;
	void RemoveAction(FunscriptAction action, bool checkInvalidSelection = true) noexcept;
	void RemoveActions(const FunscriptArray& actions) noexcept;

	std::vector<FunscriptAction> GetLastStroke(int32_t time_ms) noexcept;

	void SetActions(const FunscriptArray& override_with) noexcept;

	inline bool HasUnsavedEdits() const { return unsavedEdits; }
	inline const std::chrono::system_clock::time_point& EditTime() const { return editTime; }

	void RemoveActionsInInterval(int32_t fromMs, int32_t toMs) noexcept;

	// selection api
	void RangeExtendSelection(int32_t rangeExtend) noexcept;
	bool ToggleSelection(FunscriptAction action) noexcept;
	
	void SetSelected(FunscriptAction action, bool selected) noexcept;
	void SelectTopActions() noexcept;
	void SelectBottomActions() noexcept;
	void SelectMidActions() noexcept;
	void SelectTime(int32_t from_ms, int32_t to_ms, bool clear=true) noexcept;
	FunscriptArray GetSelection(int32_t fromMs, int32_t toMs) noexcept;

	void SelectAction(FunscriptAction select) noexcept;
	void DeselectAction(FunscriptAction deselect) noexcept;
	void SelectAll() noexcept;
	void RemoveSelectedActions() noexcept;
	void MoveSelectionTime(int32_t time_offset, float frameTimeMs) noexcept;
	void MoveSelectionPosition(int32_t pos_offset) noexcept;
	inline bool HasSelection() const noexcept { return data.selection.size() > 0; }
	inline int32_t SelectionSize() const noexcept { return data.selection.size(); }
	inline void ClearSelection() noexcept { data.selection.clear(); }
	inline const FunscriptAction* GetClosestActionSelection(int32_t time_ms) noexcept { return getActionAtTime(data.selection, time_ms, std::numeric_limits<int32_t>::max()); }
	
	void SetSelection(const FunscriptArray& action_to_select, bool unsafe) noexcept;
	bool IsSelected(FunscriptAction action) noexcept;

	void EqualizeSelection() noexcept;
	void InvertSelection() noexcept;

	FunscriptSpline ScriptSpline;
	inline const float Spline(float timeMs) noexcept {
		return ScriptSpline.Sample(data.Actions, timeMs);
	}

	inline const float SplineClamped(float timeMs) noexcept {
		return Util::Clamp<float>(Spline(timeMs) * 100.f, 0.f, 100.f);
	}
};

inline bool Funscript::open(const std::string& file)
{
	OFS_PROFILE(__FUNCTION__);
	UpdatePath(file);
	scriptOpened = true;

	{
		nlohmann::json json;
		json = Util::LoadJson(file, &scriptOpened);

		if (!scriptOpened || !json.is_object() && json["actions"].is_array()) {
			LOGF_ERROR("Failed to parse funscript. \"%s\"", file.c_str());
			return false;
		}

		Json = std::move(json);
	}
	auto actions = Json["actions"];
	data.Actions.clear();

	for (auto& action : actions) {
		int32_t time_ms = action["at"];
		int32_t pos = action["pos"];
		if (time_ms >= 0) {
			data.Actions.emplace(time_ms, pos);
		}
	}

	loadMetadata();

	NotifyActionsChanged(false);

	Json.erase("version");
	Json.erase("inverted");
	Json.erase("range");
	Json.erase("OpenFunscripter");
	Json.erase("metadata");
	return true;
}

inline void Funscript::save(const std::string& path, bool override_location)
{
	OFS_PROFILE(__FUNCTION__);
	saveMetadata();

	auto& actions = Json["actions"];
	actions.clear();

	// make sure actions are sorted
	sortActions(data.Actions);

	if (override_location) {
		CurrentPath = path;
		unsavedEdits = false;
	}

	auto copyActions = data.Actions;
	startSaveThread(path, std::move(copyActions), std::move(Json));
}