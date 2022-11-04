#ifndef  RT3GUI_TOOLSTRIP_H
#define RT3GUI_TOOLSTRIP_H

#include <imgui.h>
#include <stdint.h>
#include <texturesLoader.h>
#include <memory>
#include <map>
#include <functional>
#include <crude_logger.h>
#include <chrono>


namespace rt3gui {

	enum class EventHandle : uint64_t { Invalid };

	template <typename... Args>
	struct Event
	{
		using Delegate = std::function<void(Args...)>;

		EventHandle Add(Delegate delegate)
		{
			auto eventHandle = static_cast<EventHandle>(++m_LastHandleId);
			m_Delegates[eventHandle] = std::move(delegate);
			return eventHandle;
		}

		bool Remove(EventHandle eventHandle)
		{
			return m_Delegates.erase(eventHandle) > 0;
		}

		void Clear()
		{
			m_Delegates.clear();
		}

		template <typename... CallArgs>
		void Invoke(CallArgs&&... args)
		{
			std::vector<Delegate> delegates;
			delegates.reserve(m_Delegates.size());
			for (auto& entry : m_Delegates)
				delegates.push_back(entry.second);

			for (auto& delegate : delegates)
				delegate(args...);
		}

		EventHandle operator += (Delegate delegate) { return Add(std::move(delegate)); }
		bool        operator -= (EventHandle eventHandle) { return Remove(eventHandle); }
		template <typename... CallArgs>
		void        operator () (CallArgs&&... args) { Invoke(std::forward<CallArgs>(args)...); }

	private:
		using EventHandleType = std::underlying_type_t<EventHandle>;

		std::map<EventHandle, Delegate> m_Delegates;
		EventHandleType            m_LastHandleId = 0;
	};

	struct Action
	{
		using OnChangeEvent = Event<Action*>;
		using OnTriggeredEvent = Event<>;

		Action() = default;
		Action(std::string_view name, OnTriggeredEvent::Delegate delegate = {});

		void SetName(std::string_view name);
		const std::string& GetName() const;

		void SetEnabled(bool set);
		bool IsEnabled() const;

		void Execute();

		OnChangeEvent       OnChange;
		OnTriggeredEvent    OnTriggered;

	private:
		std::string  m_Name;
		bool    m_IsEnabled = true;
	};

	

	class Toolstrip
	{
	public:
		Toolstrip(std::shared_ptr<rt3gui::TexturesLoader> _texturesLoader, std::map<std::string, Action>& _toolstripActions);
		~Toolstrip();

		void drawToolstrip();

		void setStartState(bool state);
		void setPauseState(bool state);

		bool getStartState();
		bool getPauseState();


		void setCurrentSamplesNum(uint64_t samplesNum);
		void setMaxSamplesNum(uint64_t samplesNum);


	private:

		void drawButton(const char* name, bool* flag, ImTextureID texture, ImVec2 texSize);
		bool toolbarAction(Action& action, ImVec2 texSize);

		bool startState;
		bool pauseState;

		uint64_t currentSamplesNum;
		uint64_t maxSamplesNum;
		std::shared_ptr<rt3gui::TexturesLoader> texturesLoader;
		std::map<std::string, Action>& toolstripActions;

		std::chrono::system_clock::time_point unpauseTime;
		std::chrono::duration<double> executionTime;

		std::map<std::string, ImTextureID> buttonTextures;
		ImTextureID newFileIcon;
		ImTextureID undoIcon;
		ImTextureID redoIcon;
		ImTextureID openIcon;
		ImTextureID saveIcon;
		ImTextureID startIcon;
		ImTextureID stopIcon;
		ImTextureID pauseIcon;
		ImTextureID stepIcon;
		ImTextureID settingsIcon;
	};



} //rt3gui
#endif // ! RT3GUI_TOOLSTRIP_H

