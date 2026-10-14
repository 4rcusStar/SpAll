// ReflectPanel.cpp — reflection tree rendering, text export, struct generation.
#include "ReflectPanel.h"

#include <QCheckBox>
#include <QClipboard>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace spall {

namespace {

/// Base alignment (std140/std430) for a reflected member type name.
quint32 baseAlignment(const QString& typeName) {
    if (typeName.startsWith(QLatin1String("double")) || typeName.startsWith(QLatin1String("dvec")) ||
        typeName.startsWith(QLatin1String("dmat"))) {
        return 16;  // relaxed double layout is not assumed
    }
    if (typeName.startsWith(QLatin1String("mat"))) {
        return 16;  // mat2/mat3/mat4 all align to 16 in std140
    }
    if (typeName.startsWith(QLatin1String("vec3"))) {
        return 16;  // the classic std140 trap: vec3 aligns to 16, not 12
    }
    if (typeName.startsWith(QLatin1String("vec2"))) {
        return 8;
    }
    if (typeName.startsWith(QLatin1String("vec4"))) {
        return 16;
    }
    return 4;  // scalars, and anything we cannot classify
}

/// Map a reflected GLSL type name to a C++ type.
QString cppType(const QString& typeName) {
    static const QHash<QString, QString> table = {
        {QStringLiteral("float"), QStringLiteral("float")},
        {QStringLiteral("vec2"), QStringLiteral("glm::vec2")},
        {QStringLiteral("vec3"), QStringLiteral("glm::vec3")},
        {QStringLiteral("vec4"), QStringLiteral("glm::vec4")},
        {QStringLiteral("int"), QStringLiteral("int32_t")},
        {QStringLiteral("ivec2"), QStringLiteral("glm::ivec2")},
        {QStringLiteral("ivec3"), QStringLiteral("glm::ivec3")},
        {QStringLiteral("ivec4"), QStringLiteral("glm::ivec4")},
        {QStringLiteral("uint"), QStringLiteral("uint32_t")},
        {QStringLiteral("uvec2"), QStringLiteral("glm::uvec2")},
        {QStringLiteral("uvec3"), QStringLiteral("glm::uvec3")},
        {QStringLiteral("uvec4"), QStringLiteral("glm::uvec4")},
        {QStringLiteral("mat2"), QStringLiteral("glm::mat2")},
        {QStringLiteral("mat3"), QStringLiteral("glm::mat3")},
        {QStringLiteral("mat4"), QStringLiteral("glm::mat4")},
        {QStringLiteral("bool"), QStringLiteral("uint32_t")},
    };
    const auto it = table.constFind(typeName);
    if (it != table.constEnd()) {
        return it.value();
    }
    return QStringLiteral("/* %1 */ float").arg(typeName);
}

/// Split "float[4][2]" into {"float", "[4][2]"}.
void splitArraySuffix(const QString& typeName, QString* base, QString* suffix) {
    const int bracket = typeName.indexOf(QLatin1Char('['));
    if (bracket < 0) {
        *base = typeName;
        *suffix = QString();
        return;
    }
    *base = typeName.left(bracket);
    *suffix = typeName.mid(bracket);
}

}  // namespace

ReflectPanel::ReflectPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    auto* header = new QHBoxLayout();
    m_summaryLabel = new QLabel(this);
    header->addWidget(m_summaryLabel);
    header->addStretch(1);
    m_hideUnusedBox = new QCheckBox(tr("Hide unused variables"), this);
    m_hideUnusedBox->setToolTip(
        tr("Bindings removed by the optimizer are shown in grey; tick this to hide them."));
    header->addWidget(m_hideUnusedBox);
    layout->addLayout(header);

    m_tree = new QTreeWidget(this);
    m_tree->setColumnCount(2);
    m_tree->setHeaderLabels({tr("Item"), tr("Value")});
    m_tree->setAlternatingRowColors(true);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tree->setColumnWidth(0, 280);
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_tree->setFont(font);
    layout->addWidget(m_tree, 1);

    auto* buttons = new QHBoxLayout();
    m_copyButton = new QPushButton(tr("Copy as Text"), this);
    m_structButton = new QPushButton(tr("Generate C++ Struct"), this);
    m_structButton->setToolTip(
        tr("Generate a std140-aligned struct from the reflected offsets/padding, with a "
           "sizeof assertion."));
    buttons->addWidget(m_copyButton);
    buttons->addWidget(m_structButton);
    buttons->addStretch(1);
    layout->addLayout(buttons);

    connect(m_copyButton, &QPushButton::clicked, this, &ReflectPanel::copyAsText);
    connect(m_structButton, &QPushButton::clicked, this, &ReflectPanel::generateStructForSelection);
    connect(m_hideUnusedBox, &QCheckBox::toggled, this, [this](bool on) {
        emit hideUnusedChanged(on);
        rebuild();
    });

    setReflection(ReflectData{}, false);
}

void ReflectPanel::retranslate() {
    m_hideUnusedBox->setText(tr("Hide unused variables"));
    m_hideUnusedBox->setToolTip(
        tr("Bindings removed by the optimizer are shown in grey; tick this to hide them."));
    m_tree->setHeaderLabels({tr("Item"), tr("Value")});
    m_copyButton->setText(tr("Copy as Text"));
    m_structButton->setText(tr("Generate C++ Struct"));
    m_structButton->setToolTip(
        tr("Generate a std140-aligned struct from the reflected offsets/padding, with a "
           "sizeof assertion."));

    // The summary row and the whole tree are rebuilt from the stored reflection data.
    rebuild();
}

quint32 ReflectPanel::alignUp(quint32 value, quint32 alignment) {
    if (alignment == 0) {
        return value;
    }
    return ((value + alignment - 1) / alignment) * alignment;
}

void ReflectPanel::setReflection(const ReflectData& data, bool valid) {
    m_data = data;
    m_valid = valid;
    rebuild();
}

void ReflectPanel::rebuild() {
    m_tree->clear();

    if (!m_valid) {
        m_summaryLabel->setText(tr("No reflection data yet. It is generated after a "
                                   "successful compile."));
        auto* item = new QTreeWidgetItem(m_tree);
        item->setText(0, tr("(not compiled yet)"));
        item->setText(1, tr("Press F7 to compile the current tab"));
        m_copyButton->setEnabled(false);
        m_structButton->setEnabled(false);
        return;
    }

    m_copyButton->setEnabled(true);
    m_structButton->setEnabled(true);
    m_summaryLabel->setText(tr("<b>descriptors</b> %1 &nbsp; <b>push constants</b> %2 &nbsp; "
                               "<b>inputs</b> %3 &nbsp; <b>outputs</b> %4")
                                .arg(m_data.descriptors.size())
                                .arg(m_data.pushConstants.size())
                                .arg(m_data.inputs.size())
                                .arg(m_data.outputs.size()));

    const bool hideUnused = m_hideUnusedBox->isChecked();

    // --- execution ----------------------------------------------------------
    {
        auto* root = new QTreeWidgetItem(m_tree);
        root->setText(0, tr("Execution"));
        root->setExpanded(true);
        const auto add = [root](const QString& key, const QString& value) {
            auto* child = new QTreeWidgetItem(root);
            child->setText(0, key);
            child->setText(1, value);
        };
        add(tr("Entry point"), m_data.execution.entryPoint);
        add(tr("Execution model"), m_data.execution.executionModel);
        if (m_data.execution.localSizeX > 0 || m_data.execution.localSizeY > 0) {
            add(tr("local size"),
                QStringLiteral("%1 x %2 x %3")
                    .arg(m_data.execution.localSizeX)
                    .arg(m_data.execution.localSizeY)
                    .arg(m_data.execution.localSizeZ));
        }
        if (m_data.execution.invocations > 0) {
            add(tr("invocations"), QString::number(m_data.execution.invocations));
        }
        if (m_data.execution.outputVertices > 0) {
            add(tr("output vertices"), QString::number(m_data.execution.outputVertices));
        }
        if (!m_data.execution.executionModes.isEmpty()) {
            add(tr("execution modes"),
                m_data.execution.executionModes.join(QStringLiteral(", ")));
        }
        add(tr("Generator"), m_data.execution.generator);
    }

    // --- descriptor bindings ------------------------------------------------
    if (!m_data.descriptors.isEmpty()) {
        auto* root = new QTreeWidgetItem(m_tree);
        root->setText(0, tr("Descriptor Bindings"));
        root->setText(1, QString::number(m_data.descriptors.size()));
        root->setExpanded(true);

        for (const DescriptorBinding& binding : m_data.descriptors) {
            if (hideUnused && !binding.used) {
                continue;
            }
            auto* item = new QTreeWidgetItem(root);
            item->setText(0, QStringLiteral("set=%1 binding=%2  %3  %4")
                                 .arg(binding.set)
                                 .arg(binding.binding)
                                 .arg(binding.typeName,
                                      binding.name.isEmpty() ? tr("(unnamed)") : binding.name));
            item->setText(1, tr("%1 bytes%2%3")
                                 .arg(binding.size)
                                 .arg(binding.count > 1 ? QStringLiteral(", count=%1").arg(binding.count)
                                                        : QString())
                                 .arg(binding.used ? QString() : tr("  [unused]")));
            if (!binding.used) {
                item->setForeground(0, QColor(0x90, 0x90, 0x90));
                item->setForeground(1, QColor(0x90, 0x90, 0x90));
            }

            for (const ReflectMember& member : binding.members) {
                auto* memberItem = new QTreeWidgetItem(item);
                memberItem->setText(0, QStringLiteral("%1  %2").arg(member.typeName, member.name));
                memberItem->setText(1, QStringLiteral("offset=%1 size=%2 padded=%3")
                                          .arg(member.offset)
                                          .arg(member.size)
                                          .arg(member.paddedSize));
                for (const ReflectMember& nested : member.members) {
                    auto* nestedItem = new QTreeWidgetItem(memberItem);
                    nestedItem->setText(0, QStringLiteral("%1  %2").arg(nested.typeName, nested.name));
                    nestedItem->setText(1, QStringLiteral("offset=%1 size=%2 padded=%3")
                                              .arg(nested.offset)
                                              .arg(nested.size)
                                              .arg(nested.paddedSize));
                }
            }
        }
    }

    // --- push constants -----------------------------------------------------
    if (!m_data.pushConstants.isEmpty()) {
        auto* root = new QTreeWidgetItem(m_tree);
        root->setText(0, tr("Push Constants"));
        root->setText(1, QString::number(m_data.pushConstants.size()));
        root->setExpanded(true);
        for (const PushConstantBlock& block : m_data.pushConstants) {
            auto* item = new QTreeWidgetItem(root);
            item->setText(0, block.name);
            item->setText(1, QStringLiteral("offset=%1 size=%2").arg(block.offset).arg(block.size));
            for (const ReflectMember& member : block.members) {
                auto* memberItem = new QTreeWidgetItem(item);
                memberItem->setText(0, QStringLiteral("%1  %2").arg(member.typeName, member.name));
                memberItem->setText(1, QStringLiteral("offset=%1 size=%2 padded=%3")
                                          .arg(member.offset)
                                          .arg(member.size)
                                          .arg(member.paddedSize));
            }
        }
    }

    // --- stage IO -----------------------------------------------------------
    const auto addStageVars = [this](const QString& title, const QVector<StageVar>& vars) {
        if (vars.isEmpty()) {
            return;
        }
        auto* root = new QTreeWidgetItem(m_tree);
        root->setText(0, title);
        root->setText(1, QString::number(vars.size()));
        root->setExpanded(true);
        for (const StageVar& var : vars) {
            auto* item = new QTreeWidgetItem(root);
            const QString name = var.isBuiltin()
                                     ? QStringLiteral("%1 (builtin %2)").arg(var.builtinName,
                                                                             QString::number(var.builtin))
                                     : var.name;
            item->setText(0, QStringLiteral("loc=%1%2  %3  %4")
                                 .arg(var.location)
                                 .arg(var.component != 0 ? QStringLiteral(" comp=%1").arg(var.component)
                                                         : QString())
                                 .arg(var.typeName, name));
            item->setText(1, var.format);
        }
    };
    addStageVars(tr("Stage Inputs"), m_data.inputs);
    addStageVars(tr("Stage Outputs"), m_data.outputs);

    // --- specialization constants -------------------------------------------
    if (!m_data.specConstants.isEmpty()) {
        auto* root = new QTreeWidgetItem(m_tree);
        root->setText(0, tr("Specialization Constants"));
        root->setText(1, QString::number(m_data.specConstants.size()));
        root->setExpanded(true);
        for (const SpecConstant& constant : m_data.specConstants) {
            auto* item = new QTreeWidgetItem(root);
            item->setText(0, QStringLiteral("id=%1  %2").arg(constant.constantId).arg(constant.name));
            item->setText(1, constant.defaultValue);
        }
    }

    if (m_tree->topLevelItemCount() == 0) {
        auto* item = new QTreeWidgetItem(m_tree);
        item->setText(0, tr("(this module exposes no reflection data)"));
    }
}

void ReflectPanel::copyAsText() {
    if (!m_valid) {
        return;
    }
    QGuiApplication::clipboard()->setText(m_data.toPlainText());
    QMessageBox::information(this, tr("Copy as Text"),
                             tr("Reflection data copied to the clipboard (aligned plain-text "
                                "tables)."));
}

QString ReflectPanel::generateStruct(const QString& structName,
                                     const QVector<ReflectMember>& members) {
    if (members.isEmpty()) {
        return {};
    }

    QStringList lines;
    lines << QStringLiteral("// Generated by SpAll from SPIR-V reflection.");
    lines << QStringLiteral("// Layout follows std140: vec3 and mat* align to 16 bytes.");
    lines << QStringLiteral("struct %1").arg(structName);
    lines << QStringLiteral("{");

    quint32 cursor = 0;
    QVector<QPair<QString, quint32>> paddingFields;  // name -> bytes

    for (const ReflectMember& member : members) {
        const QString typeName = member.typeName;
        QString base;
        QString suffix;
        splitArraySuffix(typeName, &base, &suffix);

        const quint32 alignment = baseAlignment(typeName);
        // Insert explicit padding when the reflected offset is not where natural
        // packing would place the member.
        if (member.offset > cursor) {
            const quint32 gap = member.offset - cursor;
            const QString padName = QStringLiteral("_pad%1").arg(paddingFields.size());
            paddingFields.append({padName, gap});
            lines << QStringLiteral("    uint8_t %1[%2];").arg(padName).arg(gap);
            cursor = member.offset;
        }

        QString cpp = cppType(base);
        if (!suffix.isEmpty()) {
            cpp += suffix;
        }
        lines << QStringLiteral("    %1 %2;  // offset %3, size %4")
                     .arg(cpp, member.name)
                     .arg(member.offset)
                     .arg(member.size);
        cursor = member.offset + member.paddedSize;
        Q_UNUSED(alignment);
    }

    // Pad the tail so sizeof() matches the block size exactly.
    quint32 total = 0;
    for (const ReflectMember& member : members) {
        total = std::max(total, member.offset + member.paddedSize);
    }
    if (cursor < total) {
        lines << QStringLiteral("    uint8_t _padEnd[%1];").arg(total - cursor);
    }
    lines << QStringLiteral("};");
    lines << QStringLiteral("static_assert(sizeof(%1) == %2, \"%1 layout mismatch\");")
                 .arg(structName)
                 .arg(total);

    return lines.join(QLatin1Char('\n'));
}

void ReflectPanel::generateStructForSelection() {
    if (!m_valid || m_data.descriptors.isEmpty()) {
        QMessageBox::information(
            this, tr("Generate C++ Struct"),
            tr("The current module has no UBO/SSBO to generate a struct from.\n\n"
               "Compile a shader that declares a layout(set=, binding=) uniform/buffer "
               "block first."));
        return;
    }

    QStringList generated;
    for (const DescriptorBinding& binding : m_data.descriptors) {
        if (binding.members.isEmpty()) {
            continue;
        }
        QString name = binding.name;
        if (name.isEmpty()) {
            name = QStringLiteral("Block_set%1_binding%2").arg(binding.set).arg(binding.binding);
        }
        // Capitalise the first letter for a struct-ish name.
        if (!name.isEmpty()) {
            name[0] = name.at(0).toUpper();
        }
        generated << generateStruct(name, binding.members);
    }

    if (generated.isEmpty()) {
        QMessageBox::information(
            this, tr("Generate C++ Struct"),
            tr("Descriptors were found, but they have no expandable block members (usually "
               "samplers or images).\n\n"
               "Struct generation only applies to uniform buffers / storage buffers."));
        return;
    }

    QGuiApplication::clipboard()->setText(generated.join(QStringLiteral("\n\n")));
    QMessageBox::information(
        this, tr("Generate C++ Struct"),
        tr("Generated structs: %1 (copied to the clipboard).\n\n"
           "Types use glm::vec*/mat*; make sure glm is available in your project.")
            .arg(generated.size()));
}

}  // namespace spall
