// Copyright (c) 2012-2023 Wojciech Figat. All rights reserved.

#include "Screen.h"
#include "Engine.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Types/Nullable.h"
#include "Engine/Platform/Window.h"
#include "Engine/Engine/EngineService.h"
#if USE_EDITOR
#include "Editor/Editor.h"
#include "Editor/Managed/ManagedEditor.h"
#else
#include "Engine/Engine/Engine.h"
#endif

Nullable<bool> Fullscreen;
Nullable<Float2> Size;
bool CursorVisible = true;
CursorLockMode CursorLock = CursorLockMode::None;

class ScreenService : public EngineService
{
public:
    ScreenService()
        : EngineService(TEXT("Screen"), 120)
    {
    }

    void Update() override;
    void Draw() override;
};

ScreenService ScreenServiceInstance;

bool Screen::GetIsFullscreen()
{
#if USE_EDITOR
    return false;
#else
	auto win = Engine::MainWindow;
	return win ? win->IsFullscreen() : false;
#endif
}

void Screen::SetIsFullscreen(bool value)
{
    Fullscreen = value;
}

Float2 Screen::GetSize()
{
#if USE_EDITOR
    return Editor::Managed->GetGameWindowSize();
#else
	auto win = Engine::MainWindow;
	return win ? win->GetClientSize() : Float2::Zero;
#endif
}

void Screen::SetSize(const Float2& value)
{
    if (value.X <= 0 || value.Y <= 0)
    {
        LOG(Error, "Invalid Screen size to set.");
        return;
    }

    Size = value;
}

Float2 Screen::ScreenToGameViewport(const Float2& screenPos)
{
#if USE_EDITOR
    return Editor::Managed->ScreenToGameViewport(screenPos);
#else
    auto win = Engine::MainWindow;
    return win ? win->ScreenToClient(screenPos) : Float2::Minimum;
#endif
}

Float2 Screen::GameViewportToScreen(const Float2& viewportPos)
{
#if USE_EDITOR
    return Editor::Managed->GameViewportToScreen(viewportPos);
#else
    auto win = Engine::MainWindow;
    return win ? win->ClientToScreen(viewportPos) : Float2::Minimum;
#endif
}

bool Screen::GetCursorVisible()
{
    return CursorVisible;
}

void Screen::SetCursorVisible(const bool value)
{
#if USE_EDITOR
    const auto win = Editor::Managed->GetGameWindow(true);
#else
	const auto win = Engine::MainWindow;
#endif
    if (win && Engine::HasGameViewportFocus())
    {
        win->SetCursor(value ? CursorType::Default : CursorType::Hidden);
    }
    CursorVisible = value;
}

CursorLockMode Screen::GetCursorLock()
{
    return CursorLock;
}

void Screen::SetCursorLock(CursorLockMode mode)
{
#if USE_EDITOR
    const auto win = Editor::Managed->GetGameWindow(true);
#else
    const auto win = Engine::MainWindow;
#endif
    if (win && mode == CursorLockMode::Clipped)
    {
#if USE_EDITOR
        Rectangle bounds(Editor::Managed->GameViewportToScreen(Float2::Zero), Editor::Managed->GetGameWindowSize());
#else
        Rectangle bounds = win->GetClientBounds();
#endif
        win->StartClippingCursor(bounds);
    }
    else if (win && CursorLock == CursorLockMode::Clipped)
    {
        win->EndClippingCursor();
    }
    CursorLock = mode;
}

void Screen::SetGameWindowMode(GameWindowMode windowMode)
{
#if !USE_EDITOR
    switch (windowMode)
    {
    case GameWindowMode::Windowed:
        if (GetIsFullscreen())
            SetIsFullscreen(false);
#if (PLATFORM_WINDOWS)
        Engine::MainWindow->SetBorderless(false, false);
#endif
        break;
    case GameWindowMode::Fullscreen:
        SetIsFullscreen(true);
        break;
    case GameWindowMode::Borderless:
#if (PLATFORM_WINDOWS)
        Engine::MainWindow->SetBorderless(true, false);
#endif
        break;
    case GameWindowMode::FullscreenBorderless:
#if (PLATFORM_WINDOWS)
        Engine::MainWindow->SetBorderless(true, true);
#endif
        break;
    default: ;
    }
#endif
}

void ScreenService::Update()
{
#if USE_EDITOR
    // Sync current cursor state in Editor (eg. when viewport focus can change)
    Screen::SetCursorVisible(CursorVisible);
#endif
}

void ScreenService::Draw()
{
#if USE_EDITOR

    // Not supported

#else

	if (Fullscreen.HasValue())
	{
		auto win = Engine::MainWindow;
		if (win)
		{
			win->SetIsFullscreen(Fullscreen.GetValue());
		}

		Fullscreen.Reset();
	}

	if (Size.HasValue())
	{
		auto win = Engine::MainWindow;
		if (win)
		{
			win->SetClientSize(Size.GetValue());
		}

		Size.Reset();
	}

#endif
}
