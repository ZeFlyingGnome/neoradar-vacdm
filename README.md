VACDM Plugin for NeoRadar

* A sample lists.yaml is provided in src\config\lists.yaml
* The Set TOBT command does not work in the menu, use the direct command instead by modifying list.yaml. You will need to right-click in the TOBT column to set TOBT.
    - name: TOBT
      width: 35
      tagItem:
        itemName: plugin:NeoVACDM:TAG_TOBT
        leftClick: plugin:NeoVACDM:ACTION_TOBTMenu
        rightClick: plugin:NeoVACDM:ACTION_TOBTManual
* vacdm.txt config file must be the plugins sub-folder of the NeoRadar folder, an error will be displayed in NeoRadar with the expected file path if it is not found
* EXOT not properly tested 
