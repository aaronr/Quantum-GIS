; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "Quantum GIS"
!define PRODUCT_VERSION "0.6 Prerelease 1"
!define PRODUCT_PUBLISHER "QGIS Development Team"
!define PRODUCT_WEB_SITE "http://qgis.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\qgis.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "C:\Development\qgis_win32\src\qgis_win322.ico"
!define MUI_UNICON "C:\Development\qgis_win32\src\qgis_win322_uninstall.ico"
;!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\qgis.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Setup.exe"
InstallDir "$PROGRAMFILES\Quantum GIS"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "qgis.exe"
  File "..\..\qgis_win32_dist\msvcp80.dll"
  File "..\..\qgis_win32_dist\msvcr80.dll"
  File "..\..\qgis_win32_dist\msvcp70.dll"
  File "..\..\qgis_win32_dist\msvcr70.dll"
  File "..\..\qgis_win32_dist\gdal12.dll"
  File "..\..\qgis_win32_dist\qt-mtnc321.dll"
  CreateDirectory "$SMPROGRAMS\Quantum GIS"
  CreateShortCut "$SMPROGRAMS\Quantum GIS\Quantum GIS.lnk" "$INSTDIR\qgis.exe"
  CreateShortCut "$DESKTOP\Quantum GIS.lnk" "$INSTDIR\qgis.exe"
  SetOutPath "$INSTDIR\lib\qgis"
  SetOverwrite try
  File "..\..\qgis_win32_dist\lib\qgis\copyright_label.dll"
  File "..\..\qgis_win32_dist\lib\qgis\delimitedtext.dll"
  File "..\..\qgis_win32_dist\lib\qgis\delimited_text.dll"
  File "..\..\qgis_win32_dist\lib\qgis\gps_importer.dll"
  File "..\..\qgis_win32_dist\lib\qgis\gpx.dll"
  File "..\..\qgis_win32_dist\lib\qgis\grid_maker.dll"
; File "..\..\qgis_win32_dist\lib\qgis\http_server.dll"
  File "..\..\qgis_win32_dist\lib\qgis\north_arrow.dll"
  File "..\..\qgis_win32_dist\lib\qgis\ogr.dll"
  File "..\..\qgis_win32_dist\lib\qgis\postgres.dll"
  File "..\..\qgis_win32_dist\lib\qgis\scale_bar.dll"
  File "..\..\qgis_win32_dist\lib\qgis\spit.dll"
  SetOutPath "$INSTDIR\share\qgis\doc"
  File "..\..\qgis_win32_dist\share\qgis\doc\favicon.ico"
  SetOutPath "$INSTDIR\share\qgis\doc\images"
  File "..\..\qgis_win32_dist\share\qgis\doc\images\qgis_new_80pct.png"
  SetOutPath "$INSTDIR\share\qgis\doc"
  File "..\..\qgis_win32_dist\share\qgis\doc\index.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install.pdf"
  SetOutPath "$INSTDIR\share\qgis\doc\install_guide"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\contents.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\index.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\install.css"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\install.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\internals.pl"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\labels.pl"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\next.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\next_g.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node1.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node10.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node11.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node12.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node13.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node14.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node15.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node16.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node17.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node18.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node19.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node2.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node20.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node3.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node4.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node5.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node6.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node7.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node8.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\node9.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\nx_grp_g.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\prev.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\prev_g.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\up.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\up_g.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\install_guide\WARNINGS"
  SetOutPath "$INSTDIR\share\qgis\doc\plugins\delimited_text"
  File "..\..\qgis_win32_dist\share\qgis\doc\plugins\delimited_text\dialog.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\plugins\delimited_text\index.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\plugins\delimited_text\plugin_manager.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\plugins\delimited_text\toolbar_icon.png"
  SetOutPath "$INSTDIR\share\qgis\doc\plugins\geoprocessing\buffer"
  File "..\..\qgis_win32_dist\share\qgis\doc\plugins\geoprocessing\buffer\alaska_pglayer.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\plugins\geoprocessing\buffer\buffer.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\plugins\geoprocessing\buffer\bufferdialog.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\plugins\geoprocessing\buffer\buffericon.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\plugins\geoprocessing\buffer\bufferresult.png"
  File "..\..\qgis_win32_dist\share\qgis\doc\plugins\geoprocessing\buffer\index.html"
  File "..\..\qgis_win32_dist\share\qgis\doc\plugins\geoprocessing\buffer\pluginmanager.png"
  SetOutPath "$INSTDIR\share\qgis\doc"
  File "..\..\qgis_win32_dist\share\qgis\doc\qgisdoc.css"
  File "..\..\qgis_win32_dist\share\qgis\doc\README"
  File "..\..\qgis_win32_dist\share\qgis\doc\userguide.pdf"
  SetOutPath "$INSTDIR\share\qgis\i18n"
  File "..\..\qgis_win32_dist\share\qgis\i18n\qgis_de.qm"
  File "..\..\qgis_win32_dist\share\qgis\i18n\qgis_fr.qm"
  File "..\..\qgis_win32_dist\share\qgis\i18n\qgis_it.qm"
  File "..\..\qgis_win32_dist\share\qgis\i18n\qgis_nl.qm"
  File "..\..\qgis_win32_dist\share\qgis\i18n\qgis_pt_BR.qm"
  File "..\..\qgis_win32_dist\share\qgis\i18n\qgis_ru.qm"
  File "..\..\qgis_win32_dist\share\qgis\i18n\qgis_se.qm"
  File "..\..\qgis_win32_dist\share\qgis\i18n\qgis_zh_CN.qm"
  SetOutPath "$INSTDIR\share\qgis\images\icons"
  File "..\..\qgis_win32_dist\share\qgis\images\icons\inoverview.png"
  File "..\..\qgis_win32_dist\share\qgis\images\icons\no_pyramid.png"
  File "..\..\qgis_win32_dist\share\qgis\images\icons\pyramid.png"
  File "..\..\qgis_win32_dist\share\qgis\images\icons\qgis-icon.png"
  File "..\..\qgis_win32_dist\share\qgis\images\icons\qgis-icon2.png"
  File "C:\Development\qgis_win32\src\qgis_win322.ico"

  SetOutPath "$INSTDIR\share\qgis\images\north_arrows"
  File "..\..\qgis_win32_dist\share\qgis\images\north_arrows\default.png"
  SetOutPath "$INSTDIR\share\qgis\images\splash"
  File "..\..\qgis_win32_dist\share\qgis\images\splash\splash.png"
  SetOutPath "$INSTDIR\share\qgis\svg\biology"
  File "..\..\qgis_win32_dist\share\qgis\svg\biology\Coniferous.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\biology\Deciduous.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\biology\Flower.svg"
  SetOutPath "$INSTDIR\share\qgis\svg\geometric"
  File "..\..\qgis_win32_dist\share\qgis\svg\geometric\Square1.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\geometric\Square2.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\geometric\Triangle1.svg"
  SetOutPath "$INSTDIR\share\qgis\svg\gpsicons"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\anchor.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\boat.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\camera.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\car.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\city_building.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\couple.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\cross.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\deer.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\dollar.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\fish.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\flag.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\food.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\gas.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\golf.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\h.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\house.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\parachute.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\phone.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\plane.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\point.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\question.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\shipwreck.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\skier.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\skull.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\swimmer.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\table.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\teepee.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\tree.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\gpsicons\walker.svg"
  SetOutPath "$INSTDIR\share\qgis\svg\icon"
  File "..\..\qgis_win32_dist\share\qgis\svg\icon\golfcourse.svg"
  SetOutPath "$INSTDIR\share\qgis\svg\north_arrows"
  File "..\..\qgis_win32_dist\share\qgis\svg\north_arrows\default.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\north_arrows\NorthArrow1.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\north_arrows\NorthArrow2.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\north_arrows\NorthArrow3.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\north_arrows\NorthArrow4.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\north_arrows\NorthArrow5.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\north_arrows\NorthArrow6.svg"
  SetOutPath "$INSTDIR\share\qgis\svg\symbol"
  File "..\..\qgis_win32_dist\share\qgis\svg\symbol\Cross1.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\symbol\Cross2.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\symbol\Cross4.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\symbol\Cross5.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\symbol\Cross6.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\symbol\Star1.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\symbol\Star2.svg"
  File "..\..\qgis_win32_dist\share\qgis\svg\symbol\Star3.svg"
  SetOutPath "$INSTDIR\share\qgis\themes\default"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\add_all_to_overview.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\add_pg_layer.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\add_raster_layer.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\add_vector_layer.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\attribute_table.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\check_version.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\contexthelp.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\digitising_general.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\digitising_line.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\digitising_point.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\digitising_polygon.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\exit.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\export_map_server.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\fileprint.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\file_new.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\file_print.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\file_save.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\file_save_as.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\gis_plain_cursor.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\help.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\help_about.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\help_contents.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\hide_all_layers.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\home_page.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\identify.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\inline_table.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\pan.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\plugin_manager.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\preferences.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\project_open.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\project_properties.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\reload.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\remove_all_from_overview.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\save_map_image.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\select.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\show_all_layers.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\sourceforge_page.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\zoom_full.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\zoom_in.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\zoom_last.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\zoom_out.png"
  File "..\..\qgis_win32_dist\share\qgis\themes\default\zoom_selected.png"
SectionEnd

Section -AdditionalIcons
  SetOutPath $INSTDIR
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\Quantum GIS\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\Quantum GIS\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\qgis.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\qgis.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\share\qgis\themes\default\zoom_selected.png"
  Delete "$INSTDIR\share\qgis\themes\default\zoom_out.png"
  Delete "$INSTDIR\share\qgis\themes\default\zoom_last.png"
  Delete "$INSTDIR\share\qgis\themes\default\zoom_in.png"
  Delete "$INSTDIR\share\qgis\themes\default\zoom_full.png"
  Delete "$INSTDIR\share\qgis\themes\default\sourceforge_page.png"
  Delete "$INSTDIR\share\qgis\themes\default\show_all_layers.png"
  Delete "$INSTDIR\share\qgis\themes\default\select.png"
  Delete "$INSTDIR\share\qgis\themes\default\save_map_image.png"
  Delete "$INSTDIR\share\qgis\themes\default\remove_all_from_overview.png"
  Delete "$INSTDIR\share\qgis\themes\default\reload.png"
  Delete "$INSTDIR\share\qgis\themes\default\project_properties.png"
  Delete "$INSTDIR\share\qgis\themes\default\project_open.png"
  Delete "$INSTDIR\share\qgis\themes\default\preferences.png"
  Delete "$INSTDIR\share\qgis\themes\default\plugin_manager.png"
  Delete "$INSTDIR\share\qgis\themes\default\pan.png"
  Delete "$INSTDIR\share\qgis\themes\default\inline_table.png"
  Delete "$INSTDIR\share\qgis\themes\default\identify.png"
  Delete "$INSTDIR\share\qgis\themes\default\home_page.png"
  Delete "$INSTDIR\share\qgis\themes\default\hide_all_layers.png"
  Delete "$INSTDIR\share\qgis\themes\default\help_contents.png"
  Delete "$INSTDIR\share\qgis\themes\default\help_about.png"
  Delete "$INSTDIR\share\qgis\themes\default\help.png"
  Delete "$INSTDIR\share\qgis\themes\default\gis_plain_cursor.png"
  Delete "$INSTDIR\share\qgis\themes\default\file_save_as.png"
  Delete "$INSTDIR\share\qgis\themes\default\file_save.png"
  Delete "$INSTDIR\share\qgis\themes\default\file_print.png"
  Delete "$INSTDIR\share\qgis\themes\default\file_new.png"
  Delete "$INSTDIR\share\qgis\themes\default\fileprint.png"
  Delete "$INSTDIR\share\qgis\themes\default\export_map_server.png"
  Delete "$INSTDIR\share\qgis\themes\default\exit.png"
  Delete "$INSTDIR\share\qgis\themes\default\digitising_polygon.png"
  Delete "$INSTDIR\share\qgis\themes\default\digitising_point.png"
  Delete "$INSTDIR\share\qgis\themes\default\digitising_line.png"
  Delete "$INSTDIR\share\qgis\themes\default\digitising_general.png"
  Delete "$INSTDIR\share\qgis\themes\default\contexthelp.png"
  Delete "$INSTDIR\share\qgis\themes\default\check_version.png"
  Delete "$INSTDIR\share\qgis\themes\default\attribute_table.png"
  Delete "$INSTDIR\share\qgis\themes\default\add_vector_layer.png"
  Delete "$INSTDIR\share\qgis\themes\default\add_raster_layer.png"
  Delete "$INSTDIR\share\qgis\themes\default\add_pg_layer.png"
  Delete "$INSTDIR\share\qgis\themes\default\add_all_to_overview.png"
  Delete "$INSTDIR\share\qgis\svg\symbol\Star3.svg"
  Delete "$INSTDIR\share\qgis\svg\symbol\Star2.svg"
  Delete "$INSTDIR\share\qgis\svg\symbol\Star1.svg"
  Delete "$INSTDIR\share\qgis\svg\symbol\Cross6.svg"
  Delete "$INSTDIR\share\qgis\svg\symbol\Cross5.svg"
  Delete "$INSTDIR\share\qgis\svg\symbol\Cross4.svg"
  Delete "$INSTDIR\share\qgis\svg\symbol\Cross2.svg"
  Delete "$INSTDIR\share\qgis\svg\symbol\Cross1.svg"
  Delete "$INSTDIR\share\qgis\svg\north_arrows\NorthArrow6.svg"
  Delete "$INSTDIR\share\qgis\svg\north_arrows\NorthArrow5.svg"
  Delete "$INSTDIR\share\qgis\svg\north_arrows\NorthArrow4.svg"
  Delete "$INSTDIR\share\qgis\svg\north_arrows\NorthArrow3.svg"
  Delete "$INSTDIR\share\qgis\svg\north_arrows\NorthArrow2.svg"
  Delete "$INSTDIR\share\qgis\svg\north_arrows\NorthArrow1.svg"
  Delete "$INSTDIR\share\qgis\svg\north_arrows\default.svg"
  Delete "$INSTDIR\share\qgis\svg\icon\golfcourse.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\walker.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\tree.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\teepee.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\table.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\swimmer.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\skull.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\skier.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\shipwreck.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\question.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\point.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\plane.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\phone.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\parachute.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\house.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\h.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\golf.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\gas.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\food.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\flag.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\fish.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\dollar.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\deer.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\cross.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\couple.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\city_building.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\car.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\camera.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\boat.svg"
  Delete "$INSTDIR\share\qgis\svg\gpsicons\anchor.svg"
  Delete "$INSTDIR\share\qgis\svg\geometric\Triangle1.svg"
  Delete "$INSTDIR\share\qgis\svg\geometric\Square2.svg"
  Delete "$INSTDIR\share\qgis\svg\geometric\Square1.svg"
  Delete "$INSTDIR\share\qgis\svg\biology\Flower.svg"
  Delete "$INSTDIR\share\qgis\svg\biology\Deciduous.svg"
  Delete "$INSTDIR\share\qgis\svg\biology\Coniferous.svg"
  Delete "$INSTDIR\share\qgis\images\splash\splash.png"
  Delete "$INSTDIR\share\qgis\images\north_arrows\default.png"
  Delete "$INSTDIR\share\qgis\images\icons\qgis-icon2.png"
  Delete "$INSTDIR\share\qgis\images\icons\qgis_win322.ico"
  Delete "$INSTDIR\share\qgis\images\icons\qgis-icon.png"
  Delete "$INSTDIR\share\qgis\images\icons\pyramid.png"
  Delete "$INSTDIR\share\qgis\images\icons\no_pyramid.png"
  Delete "$INSTDIR\share\qgis\images\icons\inoverview.png"
  Delete "$INSTDIR\share\qgis\i18n\qgis_zh_CN.qm"
  Delete "$INSTDIR\share\qgis\i18n\qgis_se.qm"
  Delete "$INSTDIR\share\qgis\i18n\qgis_ru.qm"
  Delete "$INSTDIR\share\qgis\i18n\qgis_pt_BR.qm"
  Delete "$INSTDIR\share\qgis\i18n\qgis_nl.qm"
  Delete "$INSTDIR\share\qgis\i18n\qgis_it.qm"
  Delete "$INSTDIR\share\qgis\i18n\qgis_fr.qm"
  Delete "$INSTDIR\share\qgis\i18n\qgis_de.qm"
  Delete "$INSTDIR\share\qgis\doc\userguide.pdf"
  Delete "$INSTDIR\share\qgis\doc\README"
  Delete "$INSTDIR\share\qgis\doc\qgisdoc.css"
  Delete "$INSTDIR\share\qgis\doc\plugins\geoprocessing\buffer\pluginmanager.png"
  Delete "$INSTDIR\share\qgis\doc\plugins\geoprocessing\buffer\index.html"
  Delete "$INSTDIR\share\qgis\doc\plugins\geoprocessing\buffer\bufferresult.png"
  Delete "$INSTDIR\share\qgis\doc\plugins\geoprocessing\buffer\buffericon.png"
  Delete "$INSTDIR\share\qgis\doc\plugins\geoprocessing\buffer\bufferdialog.png"
  Delete "$INSTDIR\share\qgis\doc\plugins\geoprocessing\buffer\buffer.png"
  Delete "$INSTDIR\share\qgis\doc\plugins\geoprocessing\buffer\alaska_pglayer.png"
  Delete "$INSTDIR\share\qgis\doc\plugins\delimited_text\toolbar_icon.png"
  Delete "$INSTDIR\share\qgis\doc\plugins\delimited_text\plugin_manager.png"
  Delete "$INSTDIR\share\qgis\doc\plugins\delimited_text\index.html"
  Delete "$INSTDIR\share\qgis\doc\plugins\delimited_text\dialog.png"
  Delete "$INSTDIR\share\qgis\doc\install_guide\WARNINGS"
  Delete "$INSTDIR\share\qgis\doc\install_guide\up_g.png"
  Delete "$INSTDIR\share\qgis\doc\install_guide\up.png"
  Delete "$INSTDIR\share\qgis\doc\install_guide\prev_g.png"
  Delete "$INSTDIR\share\qgis\doc\install_guide\prev.png"
  Delete "$INSTDIR\share\qgis\doc\install_guide\nx_grp_g.png"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node9.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node8.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node7.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node6.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node5.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node4.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node3.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node20.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node2.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node19.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node18.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node17.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node16.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node15.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node14.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node13.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node12.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node11.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node10.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\node1.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\next_g.png"
  Delete "$INSTDIR\share\qgis\doc\install_guide\next.png"
  Delete "$INSTDIR\share\qgis\doc\install_guide\labels.pl"
  Delete "$INSTDIR\share\qgis\doc\install_guide\internals.pl"
  Delete "$INSTDIR\share\qgis\doc\install_guide\install.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\install.css"
  Delete "$INSTDIR\share\qgis\doc\install_guide\index.html"
  Delete "$INSTDIR\share\qgis\doc\install_guide\contents.png"
  Delete "$INSTDIR\share\qgis\doc\install.pdf"
  Delete "$INSTDIR\share\qgis\doc\index.html"
  Delete "$INSTDIR\share\qgis\doc\images\qgis_new_80pct.png"
  Delete "$INSTDIR\share\qgis\doc\favicon.ico"
  Delete "$INSTDIR\lib\qgis\spit.dll"
  Delete "$INSTDIR\lib\qgis\scale_bar.dll"
  Delete "$INSTDIR\lib\qgis\postgres.dll"
  Delete "$INSTDIR\lib\qgis\ogr.dll"
  Delete "$INSTDIR\lib\qgis\north_arrow.dll"
;  Delete "$INSTDIR\qgis\http_server.dll"
  Delete "$INSTDIR\lib\qgis\grid_maker.dll"
  Delete "$INSTDIR\lib\qgis\gpx.dll"
  Delete "$INSTDIR\lib\qgis\gps_importer.dll"
  Delete "$INSTDIR\lib\qgis\delimited_text.dll"
  Delete "$INSTDIR\lib\qgis\delimitedtext.dll"
  Delete "$INSTDIR\lib\qgis\copyright_label.dll"
  Delete "$INSTDIR\qgis.exe"
  Delete "$INSTDIR\gdal12.dll"
  Delete "$INSTDIR\qt-mtnc321.dll"
  Delete "$INSTDIR\msvcp80.dll"
  Delete "$INSTDIR\msvcr80.dll"
  Delete "$INSTDIR\msvcp70.dll"
  Delete "$INSTDIR\msvcr70.dll"

  Delete "$SMPROGRAMS\Quantum GIS\Uninstall.lnk"
  Delete "$SMPROGRAMS\Quantum GIS\Website.lnk"
  Delete "$DESKTOP\Quantum GIS.lnk"
  Delete "$SMPROGRAMS\Quantum GIS\Quantum GIS.lnk"

  RMDir "$SMPROGRAMS\Quantum GIS"
  RMDir "$INSTDIR\share\qgis\themes\default"
  RMDir "$INSTDIR\share\qgis\svg\symbol"
  RMDir "$INSTDIR\share\qgis\svg\north_arrows"
  RMDir "$INSTDIR\share\qgis\svg\icon"
  RMDir "$INSTDIR\share\qgis\svg\gpsicons"
  RMDir "$INSTDIR\share\qgis\svg\geometric"
  RMDir "$INSTDIR\share\qgis\svg\biology"
  RMDir "$INSTDIR\share\qgis\images\splash"
  RMDir "$INSTDIR\share\qgis\images\north_arrows"
  RMDir "$INSTDIR\share\qgis\images\icons"
  RMDir "$INSTDIR\share\qgis\i18n"
  RMDir "$INSTDIR\share\qgis\doc\plugins\geoprocessing\buffer"
  RMDir "$INSTDIR\share\qgis\doc\plugins\geoprocessing"
  RMDir "$INSTDIR\share\qgis\doc\plugins\delimited_text"
  RMDir "$INSTDIR\share\qgis\doc\plugins"
  RMDir "$INSTDIR\share\qgis\doc\install_guide"
  RMDir "$INSTDIR\share\qgis\doc\images"
  RMDir "$INSTDIR\share\qgis\doc"
  RMDir "$INSTDIR\share\qgis\images"
  RMDir "$INSTDIR\share\qgis\themes"
  RMDir "$INSTDIR\share\qgis\svg"
  RMDir "$INSTDIR\share\qgis"
  RMDir "$INSTDIR\share"
  RMDir "$INSTDIR\lib\qgis"
  RMDir "$INSTDIR\lib"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
