#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QStackedWidget>
#include <QVector>
#include "sidebarbutton.h"
#include "app_loader/mdiview.h"

class ModulesGenericView;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    // Get the MDI view
    MdiView* getMdiView() const { return m_mdiView; }
    
    // Refresh the core module view
    void refreshCoreModuleView();
    
    // Refresh the UI modules view
    void refreshModulesView();

private slots:
    void onSidebarButtonClicked();

private:
    void setupUi();
    void createSidebar();
    void createContentPages();
    
    QHBoxLayout *m_mainLayout;
    
    // Sidebar
    QFrame *m_sidebar;
    QVBoxLayout *m_sidebarLayout;
    QVector<SidebarButton*> m_sidebarButtons;
    
    // Content area
    QStackedWidget *m_contentStack;
    
    // MDI View
    MdiView *m_mdiView;
    
    // Modules View
    ModulesGenericView *m_modulesGenericView;
}; 