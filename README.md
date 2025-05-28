# NeoVACDM
VACDM Plugin for NeoRadar

* A sample lists.ymal is provided in src\config\lists.yaml
* The Set TOBT command does not work in the menu, use the direct command instead by modifying list.yaml:
    - name: TOBT
      width: 35
      tagItem:
        itemName: plugin:NeoVACDM:TAG_TOBT
        leftClick: plugin:NeoVACDM:ACTION_TOBTMenu
        rightClick: plugin:NeoVACDM:ACTION_TOBTManual
* Config file vacdm.txt location is hardcoded to be at the root of the NeoRadar folder, an error will be displayed in NoeRadar with the expected file path if ot is not found