#pragma once

#include "luainterface.h"

#include <QDockWidget>
#include <QCloseEvent>
#include "toolbartab.h"

namespace Ui {
	class Toolbar;
}

namespace lc {
    namespace ui {
        namespace widgets {
            /**
             * \brief Toolbar widget, displays only ToolbarTabs
             */
            class Toolbar : public QDockWidget {
                public:
                    /**
                     * \brief Create widget
                     * \param parent Pointer to parent widget
                     */
                    Toolbar(lc::ui::LuaInterface* luaInterface, QWidget* parent = 0);

                    /**
                     * \brief Destructor
                     */
                    ~Toolbar();

                    /**
                     * \brief Add a new tab
                     * \param name Tab name
                     * \param page Pointer to ToolbarTab
                     * \return Pointer to ToolbarTab
                     */
                    ToolbarTab* addTab(const char* name);

                    /**
                     * \brief Add a new tab
                     * \param newTab Pointer to ToolbarTab
                     */
                    void addTab(ToolbarTab* newTab);

                    /**
                     * \brief Remove a tab
                     * \param tab Pointer to tab widget
                     */
                    void removeTab(ToolbarTab* tab);

                    /**
                     * \brief Remove a tab
                     * \param tabName name of tab widget
                     */
                    void removeTab(const char* tabName);

                    /**
                     * \brief Get existing tab
                     * \param name Tab name
                     * \return Pointer to ToolbarTab
                     */
                    ToolbarTab* tabByName(const char* name);

                    /**
                     * \brief Close event
                     */
					void closeEvent(QCloseEvent* event);

                    /**
                     * \brief Initialize toolbar
                     */
                    void initializeToolbar(QWidget* linePatternSelect, QWidget* lineWidthSelect, QWidget* colorSelect);

                    /**
                     * \brief Add button to toolbar
                     */
                    void addButton(const char* name, const char* icon, const char* groupBox, kaguya::LuaRef cb, const char* tooltip = "", bool checkable = false, const char* tabName = "Quick Access");

                    /**
                     * \brief Remove group from quickaccesstab if it exists
                     * \param groupName Group to be removed
                     * \param tabName Group the tab is in, default Quick Access
                     */
                    void removeGroupByName(const char* groupName, const char* tabName = "Quick Access");

                private:
                    Ui::Toolbar* ui;
                    lc::ui::LuaInterface* luaInterface;
                    QMap<QString, ToolbarTab*> tabs;
            };
        }
    }
}
