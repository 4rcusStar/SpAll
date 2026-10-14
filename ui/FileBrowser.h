// FileBrowser.h — folder tree for opening shader files.
//
// Shows a whole directory tree (not just a flat list) and lets the user open any
// file whose extension this tool knows how to compile. Directories are browsable;
// unsupported files are still visible but cannot be opened, so the tree reflects
// the real folder contents instead of hiding things.
#pragma once

#include <QString>
#include <QStringList>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QFileSystemModel;
class QLabel;
class QLineEdit;
class QPushButton;
class QTreeView;
QT_END_NAMESPACE

namespace spall {

class FileBrowser : public QWidget {
    Q_OBJECT

public:
    explicit FileBrowser(QWidget* parent = nullptr);

    /// Extensions the tool can compile, without the dot: vert, frag, comp, ...
    static QStringList supportedSuffixes();
    /// A name filter string for QFileDialog, shared with the Open action so both
    /// entry points agree on what is supported.
    static QString fileDialogFilter();

    /// Point the tree at `path`. Ignored when the path does not exist.
    void setRootPath(const QString& path);
    QString rootPath() const;

    /// Select and scroll to `filePath` (used when a tab gains focus).
    void revealFile(const QString& filePath);

    /// Re-apply every user-visible string from the current catalogue.
    void retranslate();

signals:
    /// Emitted when the user activates a supported shader file.
    void fileActivated(const QString& filePath);

private slots:
    void onActivated(const QModelIndex& index);
    void onCustomContextMenu(const QPoint& position);
    void onFilterChanged(const QString& text);
    void chooseRootFolder();

private:
    /// True when `path` has an extension this tool can compile.
    static bool isSupportedFile(const QString& path);

    QFileSystemModel* m_model = nullptr;
    QTreeView* m_view = nullptr;
    QLineEdit* m_filterEdit = nullptr;
    QLabel* m_rootLabel = nullptr;
    QPushButton* m_chooseButton = nullptr;
};

}  // namespace spall
