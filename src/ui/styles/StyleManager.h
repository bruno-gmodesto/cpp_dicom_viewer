#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <QString>
#include <QWidget>

namespace ui {

class StyleManager {
public:
    static StyleManager& instance();

    struct Colors {
        static constexpr const char* background = "#0a0a0a";
        static constexpr const char* backgroundLight = "#111111";
        static constexpr const char* border = "#1a1a1a";
        static constexpr const char* textPrimary = "#ffffff";
        static constexpr const char* textSecondary = "#666666";
        static constexpr const char* textMuted = "#333333";
        static constexpr const char* accent = "#ffffff";
        static constexpr const char* accentHover = "#e0e0e0";
    };

    QString mainWindowStyle() const;
    QString sidePanelStyle() const;
    QString primaryButtonStyle() const;
    QString secondaryButtonStyle() const;
    QString labelTitleStyle() const;
    QString labelSubtitleStyle() const;
    QString labelMutedStyle() const;
    QString viewerAreaStyle() const;

    void applyGlobalStyle(QWidget* widget) const;

private:
    StyleManager() = default;
    ~StyleManager() = default;
    StyleManager(const StyleManager&) = delete;
    StyleManager& operator=(const StyleManager&) = delete;
};

} // namespace ui

#endif // STYLEMANAGER_H
