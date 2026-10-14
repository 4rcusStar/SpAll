// FileBrowser.cpp — folder tree with shader-file filtering.
#include "FileBrowser.h"

#include <QAction>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTreeView>
#include <QVBoxLayout>

#include "ShaderStage.h"

namespace spall {

namespace {

/// Settings key for the last browsed folder.
constexpr auto kRootKey = "files/browserRoot";

}  // namespace

QStringList FileBrowser::supportedSuffixes() {
    // Derived from the stage keys so the extension list cannot drift away from
    // what stageFromFileName() is able to map back to a stage.
    static const QStringList suffixes = [] {
        static const char* const keys[] = {"vert", "tesc", "tese", "geom",  "frag",
                                           "comp", "rgen", "rahit", "rchit", "rmiss",
                                           "rint", "rcall", "task", "mesh"};
        QStringList list;
        for (const char* key : keys) {
            ShaderStage stage = ShaderStage::Vertex;
            if (stageFromKey(QString::fromLatin1(key), &stage)) {
                // stageFileSuffix() includes the leading dot.
                list << stageFileSuffix(stage).mid(1);
            }
        }
        return list;
    }();
    return suffixes;
}

QString FileBrowser::fileDialogFilter() {
    QStringList patterns;
    for (const QString& suffix : supportedSuffixes()) {
        patterns << QStringLiteral("*.") + suffix;
    }
    // Shared GLSL headers are useful to open for editing even though they compile
    // as part of whatever includes them.
    patterns << QStringLiteral("*.glsl") << QStringLiteral("*.inc");

    // Static, so it cannot use the member tr(); the context is spelled out instead.
    return QCoreApplication::translate("FileBrowser",
                                       "Shaders (%1);;SPIR-V Assembly (*.spvasm);;All Files (*)")
        .arg(patterns.join(QLatin1Char(' ')));
}

FileBrowser::FileBrowser(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(3);

    // --- root row -----------------------------------------------------------
    auto* rootRow = new QHBoxLayout();
    rootRow->setSpacing(3);
    m_rootLabel = new QLabel(this);
    m_rootLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_chooseButton = new QPushButton(QStringLiteral("…"), this);
    m_chooseButton->setFixedWidth(26);
    rootRow->addWidget(m_rootLabel, 1);
    rootRow->addWidget(m_chooseButton);
    layout->addLayout(rootRow);

    // --- filter -------------------------------------------------------------
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setClearButtonEnabled(true);
    layout->addWidget(m_filterEdit);

    // --- tree ---------------------------------------------------------------
    m_model = new QFileSystemModel(this);
    m_model->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    m_model->setReadOnly(true);

    m_view = new QTreeView(this);
    m_view->setModel(m_model);
    m_view->setHeaderHidden(true);
    // Only the name column is useful in a narrow dock.
    for (int column = 1; column < m_model->columnCount(); ++column) {
        m_view->hideColumn(column);
    }
    m_view->setAnimated(false);
    m_view->setUniformRowHeights(true);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_view, 1);

    retranslate();

    connect(m_view, &QTreeView::activated, this, &FileBrowser::onActivated);
    connect(m_view, &QTreeView::doubleClicked, this, &FileBrowser::onActivated);
    connect(m_view, &QTreeView::customContextMenuRequested, this,
            &FileBrowser::onCustomContextMenu);
    connect(m_filterEdit, &QLineEdit::textChanged, this, &FileBrowser::onFilterChanged);
    connect(m_chooseButton, &QPushButton::clicked, this, &FileBrowser::chooseRootFolder);

    // Restore the last browsed folder, falling back to the current directory.
    QSettings settings;
    const QString saved = settings.value(QLatin1String(kRootKey)).toString();
    const QString initial = (!saved.isEmpty() && QFileInfo::exists(saved))
                                ? saved
                                : QDir::currentPath();
    setRootPath(initial);
}

void FileBrowser::retranslate() {
    m_rootLabel->setToolTip(tr("Folder currently being browsed"));
    m_chooseButton->setToolTip(tr("Choose the folder to browse"));
    m_filterEdit->setPlaceholderText(tr("Filter by name..."));
}

bool FileBrowser::isSupportedFile(const QString& path) {
    const QString suffix = QFileInfo(path).suffix().toLower();
    return !suffix.isEmpty() && supportedSuffixes().contains(suffix);
}

void FileBrowser::setRootPath(const QString& path) {
    const QFileInfo info(path);
    if (!info.exists() || !info.isDir()) {
        return;
    }
    const QString absolute = info.absoluteFilePath();
    m_model->setRootPath(absolute);
    m_view->setRootIndex(m_model->index(absolute));

    // Keep the label readable: show the last two path components.
    const QString name = info.fileName().isEmpty() ? absolute : info.fileName();
    m_rootLabel->setText(name);
    m_rootLabel->setToolTip(absolute);

    QSettings settings;
    settings.setValue(QLatin1String(kRootKey), absolute);
}

QString FileBrowser::rootPath() const {
    return m_model->rootPath();
}

void FileBrowser::revealFile(const QString& filePath) {
    if (filePath.isEmpty() || !QFileInfo::exists(filePath)) {
        return;
    }
    const QModelIndex index = m_model->index(filePath);
    if (!index.isValid()) {
        return;
    }
    m_view->setCurrentIndex(index);
    m_view->scrollTo(index);
}

void FileBrowser::chooseRootFolder() {
    const QString directory = QFileDialog::getExistingDirectory(
        this, tr("Choose the folder to browse"), rootPath());
    if (!directory.isEmpty()) {
        setRootPath(directory);
    }
}

void FileBrowser::onActivated(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }
    const QString path = m_model->filePath(index);
    const QFileInfo info(path);

    if (info.isDir()) {
        // Toggle expansion so a double-click behaves as expected.
        m_view->setExpanded(index, !m_view->isExpanded(index));
        return;
    }
    if (!isSupportedFile(path)) {
        QMessageBox::information(
            this, tr("Cannot Open"),
            tr("%1 is not a shader file this tool can compile.\n\nSupported "
               "extensions: %2")
                .arg(info.fileName(), supportedSuffixes().join(QLatin1String(", "))));
        return;
    }
    emit fileActivated(path);
}

void FileBrowser::onCustomContextMenu(const QPoint& position) {
    const QModelIndex index = m_view->indexAt(position);
    if (!index.isValid()) {
        return;
    }
    const QString path = m_model->filePath(index);
    const QFileInfo info(path);

    QMenu menu(this);
    if (info.isDir()) {
        QAction* browse = menu.addAction(tr("Browse This Folder"));
        connect(browse, &QAction::triggered, this, [this, path]() { setRootPath(path); });
    } else {
        QAction* open = menu.addAction(tr("Open"));
        open->setEnabled(isSupportedFile(path));
        connect(open, &QAction::triggered, this, [this, path]() { emit fileActivated(path); });
        menu.addSeparator();
        QAction* reveal = menu.addAction(tr("Reveal in Folder"));
        connect(reveal, &QAction::triggered, this,
                [this, path]() { setRootPath(QFileInfo(path).absolutePath()); });
    }
    menu.exec(m_view->viewport()->mapToGlobal(position));
}

void FileBrowser::onFilterChanged(const QString& text) {
    const QString needle = text.trimmed();
    if (needle.isEmpty()) {
        m_model->setNameFilterDisables(true);
        m_model->setNameFilters({});
        return;
    }
    // Name filters accept wildcards; wrap the text so it matches anywhere in the
    // name. Disables (greys out) non-matching entries instead of hiding them, which
    // keeps the folder structure readable.
    m_model->setNameFilters({QStringLiteral("*%1*").arg(needle)});
    m_model->setNameFilterDisables(true);
}

}  // namespace spall
