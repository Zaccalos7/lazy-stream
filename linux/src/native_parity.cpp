#include "native_window.hpp"
#include "native_controls.hpp"
#include "native_theme.hpp"
#include "version.hpp"
#include "../third_party/qrcodegen/qrcodegen.hpp"
#include <QComboBox>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStatusBar>
#include <QTableWidget>
#include <QToolButton>
#include <QUrl>
#include <QSysInfo>
#include <QStorageInfo>
#include <fstream>
#include <unistd.h>

namespace orbis {
namespace {
QString value(const Json& j) { return QString::fromStdString(j.str()); }
QLabel* label(QString text, QWidget* parent, const char* name = "muted") {
    auto result = new QLabel(text, parent); result->setObjectName(name);
    result->setTextFormat(Qt::PlainText); result->setWordWrap(true); return result;
}
void clearRows(QLayout* layout) {
    while (auto item = layout->takeAt(0)) {
        if (auto widget = item->widget()) { widget->hide(); widget->deleteLater(); }
        delete item;
    }
}
QString bitrate(double kbps) { return kbps >= 1000 ? QString::number(kbps/1000, 'g', 5) + " Mbit/s" : QString::number(kbps, 'g', 5) + " kbit/s"; }
}

void NativeWindow::buildChannels() {
    auto heading = body->findChild<QFrame*>("pageHeading");
    auto actions = new QWidget(heading); new QHBoxLayout(actions);
    addAction("addConfiguration", [this] { editChannel(); }, actions, "plus");
    heading->layout()->addWidget(actions);
    auto filters = new QWidget(body); auto row = new QHBoxLayout(filters);
    row->setContentsMargins(0,0,0,0);
    auto field = [&](const QString& title, QComboBox* pick) {
        auto col = new QVBoxLayout; col->addWidget(label(title, filters, "sectionLabel"));
        col->addWidget(pick); pick->setFixedWidth(162); row->addLayout(col);
        connect(pick, &QComboBox::currentIndexChanged, this, [this] { refreshConfigurations(); });
    };
    platformFilter = new QComboBox(filters); platformFilter->setObjectName("platformFilter");
    platformFilter->addItem("–", "");
    for (const auto& name : {"Twitch", "YouTube", "Kick", "Facebook", "TikTok", "Custom"})
        platformFilter->addItem(name == QString("Facebook") ? "Facebook Gaming" : name, name);
    field(t("platform"), platformFilter);
    statusFilter = new QComboBox(filters); statusFilter->setObjectName("activeFilter");
    statusFilter->addItem("–", ""); statusFilter->addItem(t("active"), "true"); statusFilter->addItem(t("deactive"), "false");
    field(t("active") + " / " + t("deactive"), statusFilter); row->addStretch(); bodyLayout->addWidget(filters);
    auto rows = new QWidget(body); rows->setObjectName("configurationRows");
    auto col = new QVBoxLayout(rows); col->setContentsMargins(0,0,0,0); col->setSpacing(8);
    bodyLayout->addWidget(rows); refreshConfigurations();
}

void NativeWindow::buildProfiles() {
    auto heading = body->findChild<QFrame*>("pageHeading");
    auto actions = new QWidget(heading); new QHBoxLayout(actions);
    addAction("addNewAudioVideoConfiguration", [this] { editProfile(); }, actions, "plus");
    actions->findChild<QPushButton*>("addNewAudioVideoConfiguration")->setText(t("addButton"));
    heading->layout()->addWidget(actions);
    auto rows = new QWidget(body); rows->setObjectName("configurationRows");
    auto col = new QVBoxLayout(rows); col->setContentsMargins(0,0,0,0); col->setSpacing(8);
    bodyLayout->addWidget(rows); refreshConfigurations();
}

void NativeWindow::refreshConfigurations() {
    auto rows = body->findChild<QWidget*>("configurationRows"); if (!rows) return;
    const bool channels = page == "channels";
    const auto records = studio.store.list(channels ? "channels" : "profiles");
    const auto signature = value(Json::string(records.dump())) + (platformFilter ? platformFilter->currentData().toString() : "") + (statusFilter ? statusFilter->currentData().toString() : "");
    if (configurationSignature == signature) return;
    configurationSignature = signature; clearRows(rows->layout());
    auto column = qobject_cast<QVBoxLayout*>(rows->layout()); int count = 0;
    for (size_t i=0; i<records.size(); ++i) {
        auto record = records.at(i); const bool active = record.get("isActive").flag();
        if (!channels && record.get("isDefaultConfiguration").flag()) continue;
        if (channels && !platformFilter->currentData().toString().isEmpty() && value(record.get("platform")) != platformFilter->currentData().toString()) continue;
        if (channels && !statusFilter->currentData().toString().isEmpty() && active != (statusFilter->currentData().toString()=="true")) continue;
        ++count; auto card = new QFrame(rows); card->setObjectName("configurationRow"); card->setProperty("recordId", qlonglong(record.get("id").num()));
        auto row = new QHBoxLayout(card); row->setContentsMargins(16,12,16,12); row->setSpacing(12);
        auto mark = new QLabel(card); mark->setPixmap((channels ? platformMark(value(record.get("platform"))) : studioIcon("video")).pixmap(24,24)); row->addWidget(mark);
        auto text = new QVBoxLayout; text->setSpacing(2);
        text->addWidget(label(channels ? value(record.get("channelName"))+" · "+value(record.get("platform")) : value(record.get("title")), card, "rowTitle"));
        if (channels) {
            if (!record.get("description").str().empty()) text->addWidget(label(value(record.get("description")), card));
            text->addWidget(label(value(record.get("streamUrl")), card));
        } else text->addWidget(label(t("videoCodecName")+": "+QString::fromStdString(record.get("videoCodecName").str("libx264"))+" · "+t("bitrate")+": "+bitrate(record.get("videoBitrate").num(4500))+" · "+t("audioBitrate")+": "+bitrate(record.get("audioBitrate").num(128)), card));
        row->addLayout(text, 1);
        if (channels || active) { auto badge = label(t(active ? "activeButton" : "deactive"), card); badge->setProperty("badge", active ? "ended" : "offline"); row->addWidget(badge); }
        auto edit = new QToolButton(card); edit->setObjectName("editConfiguration"); edit->setProperty("rowAction", true); edit->setIcon(studioIcon("edit")); edit->setToolTip(t(channels ? "seeDetails" : "editModalTitle")); edit->setAccessibleName(edit->toolTip()); row->addWidget(edit);
        connect(edit, &QToolButton::clicked, this, [this, channels, record] { run([&] { if (channels) editChannel(record); else editProfile(record); configurationSignature.clear(); refreshConfigurations(); }); });
        if (!channels) { auto copy = new QToolButton(card); copy->setObjectName("duplicateConfiguration"); copy->setProperty("rowAction", true); copy->setIcon(studioIcon("copy")); copy->setToolTip(t("duplicate")); copy->setAccessibleName(t("duplicate")); row->addWidget(copy); connect(copy, &QToolButton::clicked, this, [this, record] { run([&] { auto duplicate=record.clone(); duplicate.set("isActive", false); duplicate.set("title", record.get("title").str()+" ("+t("duplicate").toStdString()+")"); studio.store.save("profiles", duplicate); refreshConfigurations(); }); }); }
        auto toggle = new ToggleSwitch(card); toggle->setObjectName("toggleConfiguration"); toggle->setChecked(active); toggle->setAccessibleName(t("activeButton")); toggle->setToolTip(t(active ? "deactivateButton" : "activeButton")); row->addWidget(toggle);
        connect(toggle, &QCheckBox::clicked, this, [this, channels, record, active, toggle] { run([&] { if (channels && !confirm(active ? "deactivateConfiguration" : "activateConfigurations")) { toggle->setChecked(active); return; } auto updated=record.clone(); updated.set("isActive", !active); studio.store.save(channels ? "channels" : "profiles", updated, record.get("id").num()); refreshConfigurations(); }); });
        column->addWidget(card);
    }
    if (!count) { auto empty=new QFrame(rows); empty->setObjectName("configurationEmpty"); auto col=new QVBoxLayout(empty); col->setContentsMargins(16,40,16,40); auto icon=new QLabel(empty); icon->setPixmap(studioIcon(channels ? "channels" : "video").pixmap(28,28)); col->addWidget(icon,0,Qt::AlignHCenter); auto msg=label(t("noDataPresent"),empty); msg->setAlignment(Qt::AlignCenter); col->addWidget(msg); column->addWidget(empty); }
}

void NativeWindow::updatePager(int total) {
    if (!pageLabel) return;
    auto numbers=pageLabel->parentWidget()->findChild<QWidget*>("livePageNumbers"); if (!numbers) return;
    const int first=std::clamp(currentPage-2,0,std::max(0,total-5));
    for (int i=0;i<5;++i) { auto button=numbers->findChild<QPushButton*>(QString("livePage%1").arg(i)); const int target=first+i; button->setVisible(target<total); button->setText(QString::number(target+1)); button->setProperty("page",target); button->setChecked(target==currentPage); }
    pageLabel->parentWidget()->setVisible(total>1);
}

void NativeWindow::showOutcome(const QString& message) {
    QDialog dialog(this); dialog.setObjectName("outcomeDialog"); dialog.setWindowTitle(t("outcome")); dialog.resize(900,550);
    auto col=new QVBoxLayout(&dialog); col->addWidget(label(t("outcome"),&dialog,"dialogTitle"));
    auto text=new QPlainTextEdit(message,&dialog); text->setReadOnly(true); text->setObjectName("outcomeMessage"); col->addWidget(text,1);
    auto close=new QPushButton(t("close"),&dialog); col->addWidget(close,0,Qt::AlignRight); connect(close,&QPushButton::clicked,&dialog,&QDialog::accept); dialog.exec();
}

void NativeWindow::showErrorReport(Json history, int videoIndex) {
    QDialog dialog(this); dialog.setObjectName("errorReportDialog"); dialog.setWindowTitle(t("reportError")); dialog.resize(660,740);
    auto col=new QVBoxLayout(&dialog); col->addWidget(label(t("reportError"),&dialog,"dialogTitle"));
    const QString report=QString("OrbisStream %1 Linux\nVideo #%2 · %3\n%4\n\n%5").arg(ORBIS_VERSION).arg(history.get("id").num()).arg(value(history.get("status")),value(history.get("startedAt")),QString::fromStdString(redactStreamText(history.get("error").str(),history.get("request"))));
    Q_UNUSED(videoIndex);
    QStringList parts; QString part; int bytes=0;
    for (auto scalar:report.toUcs4()) { const QString rune=QString::fromUcs4(&scalar,1); const int size=rune.toUtf8().size(); if (bytes+size>600) { parts.append(part); part.clear(); bytes=0; } part+=rune; bytes+=size; } if (!part.isEmpty()) parts.append(part);
    auto qr=new QLabel(&dialog); qr->setObjectName("reportQr"); qr->setAlignment(Qt::AlignCenter); col->addWidget(qr,1);
    auto nav=new QWidget(&dialog); auto row=new QHBoxLayout(nav); auto previous=new QPushButton(t("back"),nav), next=new QPushButton(t("next"),nav); auto position=new QLabel(nav); row->addWidget(previous); row->addWidget(position,1,Qt::AlignCenter); row->addWidget(next); col->addWidget(nav); nav->setVisible(parts.size()>1); int index=0;
    auto render=[&] { const auto payload=QString("Orbis report %1/%2\n%3").arg(index+1).arg(parts.size()).arg(parts[index]).toUtf8(); auto code=qrcodegen::QrCode::encodeText(payload.constData(), qrcodegen::QrCode::Ecc::MEDIUM); const int scale=5, border=4; QImage image((code.getSize()+border*2)*scale,(code.getSize()+border*2)*scale,QImage::Format_RGB32); image.fill(Qt::white); QPainter painter(&image); for(int y=0;y<code.getSize();++y) for(int x=0;x<code.getSize();++x) if(code.getModule(x,y)) painter.fillRect((x+border)*scale,(y+border)*scale,scale,scale,Qt::black); qr->setPixmap(QPixmap::fromImage(image).scaled(440,440,Qt::KeepAspectRatio,Qt::FastTransformation)); position->setText(QString::number(index+1)+" / "+QString::number(parts.size())); previous->setEnabled(index>0); next->setEnabled(index+1<parts.size()); };
    connect(previous,&QPushButton::clicked,&dialog,[&]{--index;render();}); connect(next,&QPushButton::clicked,&dialog,[&]{++index;render();}); render();
    col->addWidget(label(t("reportHelp"),&dialog)); col->addWidget(label(t("reportMultiple"),&dialog)); auto issue=new QPushButton(t("reportOpenIssue"),&dialog); issue->setProperty("primary",true); col->addWidget(issue); connect(issue,&QPushButton::clicked,&dialog,[]{QDesktopServices::openUrl(QUrl("https://github.com/Zaccalos7/lazy-stream-windows/issues/new"));});
    auto expand=new QPushButton(t("reportText"),&dialog); expand->setCheckable(true); col->addWidget(expand); auto text=new QPlainTextEdit(report,&dialog); text->setObjectName("reportText"); text->setReadOnly(true); text->hide(); col->addWidget(text); connect(expand,&QPushButton::toggled,text,&QWidget::setVisible);
    auto close=new QPushButton(t("close"),&dialog); col->addWidget(close,0,Qt::AlignRight); connect(close,&QPushButton::clicked,&dialog,&QDialog::accept); dialog.exec();
}

// MainTaskManager: separate system/app rings, expandable FFmpeg processes and ordered facts.
void NativeWindow::buildMetrics() {
    auto container=body->findChild<QWidget*>("metricsContainer");
    if (!container) {
        container=new QWidget(body); container->setObjectName("metricsContainer"); auto col=new QVBoxLayout(container); col->setContentsMargins(0,0,0,0); col->setSpacing(24); bodyLayout->addWidget(container);
        auto meters=[&](const char* name,const QList<QPair<QString,QString>>& entries) {
            auto panel=new QWidget(container); panel->setObjectName(name); auto grid=new QGridLayout(panel); grid->setContentsMargins(0,0,0,0); grid->setSpacing(12);
            for(int i=0;i<entries.size();++i) {
                auto card=new QFrame(panel); card->setObjectName("meterCard"); auto content=new QVBoxLayout(card); content->setContentsMargins(14,20,14,22); content->setSpacing(14);
                auto heading=label(entries[i].second,card,"sectionLabel"); heading->setAlignment(Qt::AlignCenter); content->addWidget(heading);
                auto ring=new MeterRing(entries[i].first.contains("Temp") ? " °C" : "%",card); ring->setObjectName(entries[i].first); content->addWidget(ring,0,Qt::AlignCenter);
                grid->addWidget(card,i/4,i%4);
            } col->addWidget(panel);
        };
        meters("systemMeters",{{"cpu","CPU"},{"ram","RAM"},{"swap","SWAP"},{"cpuTemp",t("TEMPERATURA_CPU")},{"gpuTemp",t("TEMPERATURA_GPU")}});
        auto heading=new QFrame(container); heading->setObjectName("pageHeading"); auto line=new QHBoxLayout(heading); line->addWidget(label(t("appUsage"),heading,"cardTitle")); col->addWidget(heading);
        meters("appMeters",{{"appCpu","CPU"},{"appRam","RAM"},{"appDisk","DISCO"},{"appNetwork","RETE"}});
        auto expand=new QPushButton(t("ffmpegProcessesDetails"),container); expand->setObjectName("ffmpegProcessesDetails"); expand->setCheckable(true); expand->setIcon(studioIcon("next")); col->addWidget(expand);
        auto processes=new QTableWidget(0,5,container); processes->setObjectName("ffmpegProcesses"); processes->setHorizontalHeaderLabels({"PID","CPU","RAM",t("threads"),t("liveManagement")}); processes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); processes->verticalHeader()->hide(); processes->setEditTriggers(QAbstractItemView::NoEditTriggers); processes->hide(); col->addWidget(processes); connect(expand,&QPushButton::toggled,processes,&QWidget::setVisible);
        auto facts=new QFrame(container); facts->setObjectName("systemInfo"); auto form=new QFormLayout(facts); form->addRow(label(t("systemInfo"),facts,"cardTitle"));
        for(const auto& key: {"os","architecture","computer","cpuModel","cpuCores","ramTotal","ramUsed","disk","diskFree","uptime","runtime"}) { auto field=label({},facts); field->setObjectName(QString("fact_")+key); form->addRow(t(key),field); } col->addWidget(facts);
    }
    const auto metrics=state.get("metrics"), app=state.get("process");
    auto ring=[&](const char* key,double amount) { container->findChild<MeterRing*>(key)->setValue(amount); };
    ring("cpu",metrics.get("cpu").num(-1)); ring("ram",metrics.get("ram").num(-1)); ring("swap",metrics.get("swap").num(-1)); ring("cpuTemp",metrics.get("cpuTemperature").num(-1)); ring("gpuTemp",metrics.get("gpuTemperature").num(-1)); ring("appCpu",app.get("cpu").num(-1)); ring("appRam",metrics.get("memoryBytes").num()>0 ? app.get("rssBytes").num()*100./metrics.get("memoryBytes").num() : -1); ring("appDisk",-1); ring("appNetwork",-1);
    for(const auto& key:{"cpuTemp","gpuTemp"}) container->findChild<MeterRing*>(key)->parentWidget()->setVisible(container->findChild<MeterRing*>(key)->value()>=0);
    auto processes=container->findChild<QTableWidget*>("ffmpegProcesses"); processes->setRowCount(0); const auto live=state.get("live");
    for(size_t i=0;i<live.size();++i) for(const auto& key:{"producerMetrics","publisherMetrics"}) { const auto process=live.at(i).get(key); if(process.get("pid").num()<=0) continue; int row=processes->rowCount(); processes->insertRow(row); const QStringList cells={QString::number(process.get("pid").num()),QString::number(process.get("cpu").num(),'f',1)+"%",QString::number(process.get("rssBytes").num()/1048576.,'f',1)+" MiB",QString::number(process.get("threads").num()),value(live.at(i).get("channelName"))}; for(int c=0;c<cells.size();++c) processes->setItem(row,c,new QTableWidgetItem(cells[c])); }
    auto fact=[&](const char* key,const QString& text) {container->findChild<QLabel*>(QString("fact_")+key)->setText(text);};
    fact("os",QSysInfo::prettyProductName()); fact("architecture",QSysInfo::currentCpuArchitecture()); fact("computer",QSysInfo::machineHostName()); fact("cpuCores",QString::number(std::thread::hardware_concurrency()));
    std::ifstream cpu("/proc/cpuinfo"); std::string line; while(std::getline(cpu,line)) if(line.starts_with("model name")) {fact("cpuModel",QString::fromStdString(line.substr(line.find(':')+1)).trimmed());break;}
    auto gib=[](double bytes){return QString::number(bytes/1073741824.,'f',1)+" GiB";}; fact("ramTotal",gib(metrics.get("memoryBytes").num())); fact("ramUsed",gib(metrics.get("memoryBytes").num()*metrics.get("ram").num()/100)); fact("disk",gib(metrics.get("diskTotal").num())); fact("diskFree",gib(metrics.get("diskAvailable").num())); fact("uptime",QString::number(int(metrics.get("uptime").num()/3600))+" h"); fact("runtime",QString("C++20 · Qt %1").arg(qVersion()));
}
} // namespace orbis
