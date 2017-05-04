Tools For Generating DT Instance
================================

DT instance, which is defined in [TR-106] Annex B., is an XML document that conforms to the DT Schema and to any additional rules specified in or referenced by the DT Schema. This folder contains tools to generate template of DT instance.

Files
-----
  - **Makefile**: Makefile for generate DT instance templates.
  - **cwmp.zip**: Archived TR-069 related schemas. This file was downloaded from <http://broadband-forum.org/cwmp.zip>.
  - **report.pl**: Script to generate TR-069 Data Model report.
  - **report.txt**: Manual for report.pl.
  - **README.md**: This file.


Usage
-----
1. Add your parameters or objects and implement their get/set functions. And make sure parameters and objects work properly.
2. Add the profile you implemented to *Makefile*. There are following 2 situations.
    - Add the profile to existed Device Type instance.
    - Add the profile to new Device Type instance.
3. Use ```make``` to generate templates. They are .xml Device Type instance documents.
4. Open the generated template which contains the profiles you have added. Then copy the parameters and objects you implemented to the real DT instance Document, which is located at **cwmp-tr069/cwmpClient/dt_doc/**. If you created a new DT instance document, just copy it to **cwmp-tr069/cwmpClient/dt_doc/** and remove **"-template"** from file name.
5. If your implementation of parameters' or objects' attributes are different from default. Please update them in the real DT instance document.
6. If you added a new DT instance, don't forget ot add a new **Device.DeviceInfo.SupportedDataModel.{i}**  object. Implementation of Supported data model objects is in  **cwmp-tr069/cwmpClient/tr181/prmt_deviceinfo.c**.


Other Notes
-----------
#### Update cwmp.zip ####
If you implented a new TR-XXX which is not in **cwmp.zip**. You can just use ```make cwmp_update``` to download new cwmp.zip.


  [TR-106]: http://www.broadband-forum.org/technical/download/TR-106_Amendment-6.pdf 

