#include "configdialog.h"
#include "ui_configdialog.h"
#include "TA3D_NameSpace.h"
#include <misc/settings.h>

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ConfigDialog::do_config()
{
    using namespace TA3D;
    // Load current settings
    TA3D::Settings::Load();

    ui->cb_display_mode->setCurrentText(QString("%1x%2 %3bits").arg(lp_CONFIG->screen_width).arg(lp_CONFIG->screen_height).arg(lp_CONFIG->color_depth));

    ui->cb_AA->setCurrentText(QString::number(lp_CONFIG->fsaa));
    ui->cb_anisotropy->setCurrentText(QString::number(lp_CONFIG->anisotropy));
    ui->cb_shadows_quality->setCurrentIndex(lp_CONFIG->shadow_quality);
    ui->cb_shadowmap_size->setCurrentIndex(lp_CONFIG->shadowmap_size);

    ui->cb_water_quality->setCurrentIndex(lp_CONFIG->water_quality);

    ui->cb_texture_cache->setChecked(lp_CONFIG->use_texture_cache);
    ui->cb_texture_compression->setChecked(lp_CONFIG->use_texture_compression);
    ui->cb_fullscreen->setChecked(lp_CONFIG->fullscreen);
    ui->cb_farsight->setChecked(lp_CONFIG->far_sight);

    // The audio tab
    ui->sb_sound_volume->setValue(lp_CONFIG->sound_volume);
    ui->sb_music_volume->setValue(lp_CONFIG->music_volume);

    // The game tab
    ui->le_player_name->setText(lp_CONFIG->player_name.c_str());
    ui->cb_language->setCurrentText(lp_CONFIG->Lang.c_str());
    ui->le_mod->setText(lp_CONFIG->last_MOD.c_str());

    // The advanced tab
    ui->cb_dev_mode->setChecked(lp_CONFIG->developerMode);
    ui->le_7z_command->setText(lp_CONFIG->system7zCommand.c_str());
    ui->le_netserver->setText(lp_CONFIG->net_server.c_str());

    // Run the config window
    exec();

    if (result() == Accepted)
    {
        switch(ui->cb_display_mode->currentIndex())
        {
#define MODE(ID, W, H, BPP)\
        case ID:\
            lp_CONFIG->screen_width = static_cast<TA3D::uint16>(W);\
            lp_CONFIG->screen_height = static_cast<TA3D::uint16>(H);\
            lp_CONFIG->color_depth = static_cast<TA3D::uint8>(BPP);\
            break
        MODE(0, 640, 480, 16);
        MODE(1, 640, 480, 32);
        MODE(2, 800, 600, 16);
        MODE(3, 800, 600, 32);
        MODE(4, 1024, 768, 16);
        MODE(5, 1024, 768, 32);
        MODE(6, 1280, 720, 16);
        MODE(7, 1280, 720, 32);
        MODE(8, 1280, 800, 16);
        MODE(9, 1280, 800, 32);
        MODE(10, 1280, 960, 16);
        MODE(11, 1280, 960, 32);
        MODE(12, 1280, 1024, 16);
        MODE(13, 1280, 1024, 32);
        MODE(14, 1440, 900, 16);
        MODE(15, 1440, 900, 32);
        MODE(16, 1600, 1200, 16);
        MODE(17, 1600, 1200, 32);
        MODE(18, 1920, 1080, 16);
        MODE(19, 1920, 1080, 32);
        MODE(20, 2560, 1600, 16);
        MODE(21, 2560, 1600, 32);
        MODE(22, 3280, 2160, 16);
        MODE(23, 3280, 2160, 32);
        }

        const int AA[] = { 1, 2, 4, 8, 16, 32 };
        lp_CONFIG->fsaa = static_cast<TA3D::sint16>(AA[ui->cb_AA->currentIndex()]);
        lp_CONFIG->fullscreen = ui->cb_fullscreen->isChecked();
        lp_CONFIG->anisotropy = static_cast<TA3D::sint16>(ui->cb_anisotropy->currentText().toInt());
        lp_CONFIG->shadow_quality = static_cast<TA3D::sint16>(ui->cb_shadows_quality->currentIndex());
        lp_CONFIG->water_quality = static_cast<TA3D::sint16>(ui->cb_water_quality->currentIndex());
        lp_CONFIG->shadowmap_size = static_cast<TA3D::uint8>(ui->cb_shadowmap_size->currentIndex());
        lp_CONFIG->use_texture_cache = ui->cb_texture_cache->isChecked();
        lp_CONFIG->use_texture_compression = ui->cb_texture_compression->isChecked();
        lp_CONFIG->far_sight = ui->cb_farsight->isChecked();

        lp_CONFIG->sound_volume = static_cast<int>(ui->sb_sound_volume->value());
        lp_CONFIG->music_volume = static_cast<int>(ui->sb_music_volume->value());

        lp_CONFIG->player_name = ui->le_player_name->text().toStdString();
        lp_CONFIG->Lang = ui->cb_language->currentText().toStdString();
        lp_CONFIG->last_MOD = ui->le_mod->text().toStdString();
        TA3D_CURRENT_MOD = lp_CONFIG->last_MOD;

        lp_CONFIG->developerMode = ui->cb_dev_mode->isChecked();

        lp_CONFIG->system7zCommand = ui->le_7z_command->text().toStdString();
        lp_CONFIG->net_server = ui->le_netserver->text().toStdString();

        // Save modified settings
        TA3D::Settings::Save();
    }
}
