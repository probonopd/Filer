/********************************************************************************
** Form generated from reading UI file 'InfoDialogHvDXib.ui'
**
** Created by: Qt User Interface Compiler version 5.15.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef INFODIALOGHVDXIB_H
#define INFODIALOGHVDXIB_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_InfoDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *typeInfo;
    QLabel *modifiedInfo;
    QLabel *versionInfo;
    QLabel *copyrightLabel;
    QLabel *iconLabel;
    QLabel *commentsLabel;
    QLabel *pathInfo;
    QLabel *openWithInfo;
    QLabel *createdLabel;
    QLabel *permissionsInfo;
    QLabel *typeLabel;
    QLabel *sizeInfo;
    QLabel *modifiedLabel;
    QLabel *iconInfo;
    QLabel *versionLabell;
    QLabel *copyrightInfo;
    QLabel *openWithLabel;
    QLabel *permissionsLabel;
    QCheckBox *executableCheckBox;
    QLabel *createdInfo;
    QLabel *sizeLabel;
    QLabel *pathLabel;
    QPushButton *changeOpenWithButton;
    QPlainTextEdit *plainTextEdit;

    void setupUi(QDialog *InfoDialog)
    {
        if (InfoDialog->objectName().isEmpty())
            InfoDialog->setObjectName(QString::fromUtf8("InfoDialog"));
        InfoDialog->resize(446, 506);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(InfoDialog->sizePolicy().hasHeightForWidth());
        InfoDialog->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(InfoDialog);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        typeInfo = new QLabel(InfoDialog);
        typeInfo->setObjectName(QString::fromUtf8("typeInfo"));
        QFont font;
        font.setPointSize(8);
        typeInfo->setFont(font);

        gridLayout->addWidget(typeInfo, 7, 1, 1, 1);

        modifiedInfo = new QLabel(InfoDialog);
        modifiedInfo->setObjectName(QString::fromUtf8("modifiedInfo"));
        modifiedInfo->setFont(font);

        gridLayout->addWidget(modifiedInfo, 4, 1, 1, 1);

        versionInfo = new QLabel(InfoDialog);
        versionInfo->setObjectName(QString::fromUtf8("versionInfo"));

        gridLayout->addWidget(versionInfo, 10, 1, 1, 1);

        copyrightLabel = new QLabel(InfoDialog);
        copyrightLabel->setObjectName(QString::fromUtf8("copyrightLabel"));
        copyrightLabel->setFont(font);
        copyrightLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(copyrightLabel, 11, 0, 1, 1);

        iconLabel = new QLabel(InfoDialog);
        iconLabel->setObjectName(QString::fromUtf8("iconLabel"));
        iconLabel->setFont(font);
        iconLabel->setAlignment(Qt::AlignBottom|Qt::AlignRight|Qt::AlignTrailing);

        gridLayout->addWidget(iconLabel, 0, 0, 1, 1);

        commentsLabel = new QLabel(InfoDialog);
        commentsLabel->setObjectName(QString::fromUtf8("commentsLabel"));
        commentsLabel->setFont(font);
        commentsLabel->setAlignment(Qt::AlignRight|Qt::AlignTop|Qt::AlignTrailing);

        gridLayout->addWidget(commentsLabel, 13, 0, 1, 1);

        pathInfo = new QLabel(InfoDialog);
        pathInfo->setObjectName(QString::fromUtf8("pathInfo"));
        pathInfo->setFont(font);

        gridLayout->addWidget(pathInfo, 1, 1, 1, 1);

        openWithInfo = new QLabel(InfoDialog);
        openWithInfo->setObjectName(QString::fromUtf8("openWithInfo"));
        openWithInfo->setFont(font);

        gridLayout->addWidget(openWithInfo, 8, 1, 1, 1);

        createdLabel = new QLabel(InfoDialog);
        createdLabel->setObjectName(QString::fromUtf8("createdLabel"));
        createdLabel->setFont(font);
        createdLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(createdLabel, 3, 0, 1, 1);

        permissionsInfo = new QLabel(InfoDialog);
        permissionsInfo->setObjectName(QString::fromUtf8("permissionsInfo"));
        permissionsInfo->setFont(font);

        gridLayout->addWidget(permissionsInfo, 5, 1, 1, 1);

        typeLabel = new QLabel(InfoDialog);
        typeLabel->setObjectName(QString::fromUtf8("typeLabel"));
        typeLabel->setFont(font);
        typeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(typeLabel, 7, 0, 1, 1);

        sizeInfo = new QLabel(InfoDialog);
        sizeInfo->setObjectName(QString::fromUtf8("sizeInfo"));
        sizeInfo->setFont(font);

        gridLayout->addWidget(sizeInfo, 2, 1, 1, 1);

        modifiedLabel = new QLabel(InfoDialog);
        modifiedLabel->setObjectName(QString::fromUtf8("modifiedLabel"));
        modifiedLabel->setFont(font);
        modifiedLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(modifiedLabel, 4, 0, 1, 1);

        iconInfo = new QLabel(InfoDialog);
        iconInfo->setObjectName(QString::fromUtf8("iconInfo"));
        iconInfo->setMinimumSize(QSize(0, 128));
        iconInfo->setFont(font);

        gridLayout->addWidget(iconInfo, 0, 1, 1, 1);

        versionLabell = new QLabel(InfoDialog);
        versionLabell->setObjectName(QString::fromUtf8("versionLabell"));
        versionLabell->setFont(font);
        versionLabell->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(versionLabell, 10, 0, 1, 1);

        copyrightInfo = new QLabel(InfoDialog);
        copyrightInfo->setObjectName(QString::fromUtf8("copyrightInfo"));

        gridLayout->addWidget(copyrightInfo, 11, 1, 1, 1);

        openWithLabel = new QLabel(InfoDialog);
        openWithLabel->setObjectName(QString::fromUtf8("openWithLabel"));
        openWithLabel->setFont(font);
        openWithLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(openWithLabel, 8, 0, 1, 1);

        permissionsLabel = new QLabel(InfoDialog);
        permissionsLabel->setObjectName(QString::fromUtf8("permissionsLabel"));
        permissionsLabel->setFont(font);
        permissionsLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(permissionsLabel, 5, 0, 1, 1);

        executableCheckBox = new QCheckBox(InfoDialog);
        executableCheckBox->setObjectName(QString::fromUtf8("executableCheckBox"));
        executableCheckBox->setFont(font);

        gridLayout->addWidget(executableCheckBox, 6, 1, 1, 1);

        createdInfo = new QLabel(InfoDialog);
        createdInfo->setObjectName(QString::fromUtf8("createdInfo"));
        createdInfo->setFont(font);

        gridLayout->addWidget(createdInfo, 3, 1, 1, 1);

        sizeLabel = new QLabel(InfoDialog);
        sizeLabel->setObjectName(QString::fromUtf8("sizeLabel"));
        sizeLabel->setFont(font);
        sizeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(sizeLabel, 2, 0, 1, 1);

        pathLabel = new QLabel(InfoDialog);
        pathLabel->setObjectName(QString::fromUtf8("pathLabel"));
        pathLabel->setFont(font);
        pathLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(pathLabel, 1, 0, 1, 1);

        changeOpenWithButton = new QPushButton(InfoDialog);
        changeOpenWithButton->setObjectName(QString::fromUtf8("changeOpenWithButton"));
        QFont font1;
        font1.setPointSize(12);
        changeOpenWithButton->setFont(font1);
        changeOpenWithButton->setAutoDefault(false);
        changeOpenWithButton->setFlat(false);

        gridLayout->addWidget(changeOpenWithButton, 9, 1, 1, 1);

        plainTextEdit = new QPlainTextEdit(InfoDialog);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(plainTextEdit->sizePolicy().hasHeightForWidth());
        plainTextEdit->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(plainTextEdit, 13, 1, 1, 1);


        retranslateUi(InfoDialog);

        QMetaObject::connectSlotsByName(InfoDialog);
    } // setupUi

    void retranslateUi(QDialog *InfoDialog)
    {
        InfoDialog->setWindowTitle(QCoreApplication::translate("InfoDialog", "Dialog", nullptr));
        versionInfo->setText(QString());
        copyrightLabel->setText(QCoreApplication::translate("InfoDialog", "Copyright:", nullptr));
        iconLabel->setText(QCoreApplication::translate("InfoDialog", "Icon:", nullptr));
        commentsLabel->setText(QCoreApplication::translate("InfoDialog", "Comments:", nullptr));
        createdLabel->setText(QCoreApplication::translate("InfoDialog", "Created:", nullptr));
        typeLabel->setText(QCoreApplication::translate("InfoDialog", "Type:", nullptr));
        modifiedLabel->setText(QCoreApplication::translate("InfoDialog", "Modified:", nullptr));
        iconInfo->setText(QString());
        versionLabell->setText(QCoreApplication::translate("InfoDialog", "Version:", nullptr));
        copyrightInfo->setText(QString());
        openWithLabel->setText(QCoreApplication::translate("InfoDialog", "Open With:", nullptr));
        permissionsLabel->setText(QCoreApplication::translate("InfoDialog", "Permissions:", nullptr));
        executableCheckBox->setText(QCoreApplication::translate("InfoDialog", "Execute as an application", nullptr));
        sizeLabel->setText(QCoreApplication::translate("InfoDialog", "Size:", nullptr));
        pathLabel->setText(QCoreApplication::translate("InfoDialog", "Path:", nullptr));
        changeOpenWithButton->setText(QCoreApplication::translate("InfoDialog", "Change...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class InfoDialog: public Ui_InfoDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // INFODIALOGHVDXIB_H
