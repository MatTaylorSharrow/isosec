#include "auditclientcpp.h"
#include "ui_auditclientcpp.h"



/**
 * Constructor
 */
AuditClientCPP::AuditClientCPP(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::AuditClientCPP), // construct on heap
    m_app(new App()) //construct App on heap (lifetime managed by QScopedPointer)
{
    m_ui->setupUi(this);
    m_app->setupApp(this);
}

/**
 * Destructor
 */
AuditClientCPP::~AuditClientCPP() = default;


/**
 * 
 */
void AuditClientCPP::printToTextArea(std::string s) {
    m_ui->textEdit->insertPlainText(QString::fromStdString(s + "\n" ));
}

/**
 *  
 */
void AuditClientCPP::on_simulateStampedeButton_clicked() 
{
    m_app->emitStampede();
}

/**
 * 
 */
void AuditClientCPP::on_simulateQueueButton_clicked()
{
    m_ui->centralwidget->setStyleSheet("background-color:yellow;");
}

