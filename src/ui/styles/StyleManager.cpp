#include "StyleManager.h"

namespace ui {

StyleManager& StyleManager::instance()
{
    static StyleManager instance;
    return instance;
}

QString StyleManager::mainWindowStyle() const
{
    return QString(R"(
        QMainWindow {
            background-color: %1;
        }
    )").arg(Colors::background);
}

QString StyleManager::sidePanelStyle() const
{
    return QString(R"(
        QWidget#sidePanel {
            background-color: %1;
            border-right: 1px solid %2;
        }
    )").arg(Colors::backgroundLight, Colors::border);
}

QString StyleManager::primaryButtonStyle() const
{
    return QString(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 6px;
            font-size: 13px;
            font-weight: 500;
            padding: 14px 24px;
            letter-spacing: 1px;
        }
        QPushButton:hover {
            background-color: %3;
        }
        QPushButton:pressed {
            background-color: #cccccc;
        }
    )").arg(Colors::accent, Colors::background, Colors::accentHover);
}

QString StyleManager::secondaryButtonStyle() const
{
    return QString(R"(
        QPushButton {
            background-color: transparent;
            color: %1;
            border: 1px solid %2;
            border-radius: 6px;
            font-size: 13px;
            font-weight: 500;
            padding: 14px 24px;
            letter-spacing: 1px;
        }
        QPushButton:hover {
            color: %3;
            border-color: #555555;
        }
        QPushButton:pressed {
            background-color: %4;
        }
    )").arg(Colors::textSecondary, Colors::textMuted, Colors::textPrimary, Colors::border);
}

QString StyleManager::labelTitleStyle() const
{
    return QString(R"(
        QLabel {
            color: %1;
            font-size: 24px;
            font-weight: 300;
            letter-spacing: 4px;
        }
    )").arg(Colors::textPrimary);
}

QString StyleManager::labelSubtitleStyle() const
{
    return QString(R"(
        QLabel {
            color: %1;
            font-size: 11px;
            font-weight: 400;
            letter-spacing: 2px;
        }
    )").arg(Colors::textSecondary);
}

QString StyleManager::labelMutedStyle() const
{
    return QString(R"(
        QLabel {
            color: %1;
            font-size: 10px;
            letter-spacing: 1px;
        }
    )").arg(Colors::textMuted);
}

QString StyleManager::viewerAreaStyle() const
{
    return QString(R"(
        QWidget#viewerArea {
            background-color: %1;
        }
        QLabel {
            color: #2a2a2a;
            font-size: 14px;
            font-weight: 300;
            letter-spacing: 2px;
        }
    )").arg(Colors::background);
}

void StyleManager::applyGlobalStyle(QWidget* widget) const
{
    if (widget) {
        widget->setStyleSheet(mainWindowStyle());
    }
}

} // namespace ui
