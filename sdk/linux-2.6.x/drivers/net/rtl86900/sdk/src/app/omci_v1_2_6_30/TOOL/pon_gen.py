#!/usr/bin/env python
import re, sys

nodes = []
KEY_ITEM = 'Name'
MGMTNAME = ''
DEST_DIR_FILE = '../MIB/src/tables'
DEST_DIR_HEADER = '../MIB/inc'

PREFIX_COMMENT = '/* \n' + \
                 ' * Copyright (C) 2014 Realtek Semiconductor Corp.\n' + \
		 ' * All Rights Reserved.\n' + \
		 ' * \n' + \
		 ' * This program is the proprietary software of Realtek Semiconductor\n' + \
		 ' * Corporation and/or its licensors, and only be used, duplicated,\n' + \
		 ' * modified or distributed under the authorized license from Realtek.\n' + \
		 ' * \n' + \
		 ' * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER\n' + \
		 ' * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.\n' + \
		 ' * \n' + \
		 ' */\n\n\n'

FUNC_TITLE = PREFIX_COMMENT + \
	     '#include "app_basic.h"\n\n'

HEADER_TITLE = PREFIX_COMMENT + \
	     '#ifndef __MIB_%s_TABLE_H__\n' + \
	     '#define __MIB_%s_TABLE_H__\n\n'

VALID_TYPE = {  '0' : 'FALSE',
                '1' : 'TRUE',
             }

INIT_TYPE = {   'OLT' : 'OMCI_ME_INIT_TYPE_OLT',
                'AUTO': 'OMCI_ME_INIT_TYPE_ONU',
            }

OLTACC_TYPE = { 'W' : 'OMCI_ME_ATTR_ACCESS_READ | OMCI_ME_ATTR_ACCESS_WRITE',
                'R' : 'OMCI_ME_ATTR_ACCESS_READ',
		'CW' : 'OMCI_ME_ATTR_ACCESS_READ | OMCI_ME_ATTR_ACCESS_WRITE | OMCI_ME_ATTR_ACCESS_SBC',
		'CR' : 'OMCI_ME_ATTR_ACCESS_READ | OMCI_ME_ATTR_ACCESS_SBC',
              }

OPTION_TYPE = { 'N' : 'OMCI_ME_ATTR_TYPE_O_NOT_SUPPORT',
                'S' : 'OMCI_ME_ATTR_TYPE_OPTIONAL',
                'M' : 'OMCI_ME_ATTR_TYPE_MANDATORY',
                'P' : 'OMCI_ME_ATTR_TYPE_PRIVATE',
              }

OUTSTYLE_TYPE = { 'CHAR' : '%s',
		  'HEX' : '0x%02x',
		  'DEC' : '%d',
		}

## A class of parse tree node
class attrNode:
    TOKENS = [ KEY_ITEM, 'Desc', 'Type', 'Len', 'IsIndex', 'MibSave',
            'OutStyle', 'OltAcc', 'AvcFlag', 'OptionType', 'DefaultValue' ]
    TYPES = { 'UIN32'      : 'UINT32',
              'UIN16'      : 'UINT16',
            }
    ## Constructor.
    def __init__(self, name):
        self.Name = name.strip()
        return

def attr_parser(num, key, val):
    node = nodes[num]

    if not key in attrNode.TOKENS:
        print 'attr invalid input %d %s %s' % (num, key, val)
        sys.exit(-1)

    setattr(node, key.strip(), val.strip())

    return

class mgmtNode:
    TOKENS = [ KEY_ITEM, 'Desc', 'MaxEntry', 'ClassId', 'InitType', 'StdType', 'ActionType' ]
    ACTION_TYPES = {"Create"    : "OMCI_MSG_TYPE_CREATE",
                    "Delete"    : "OMCI_MSG_TYPE_DELETE",
                    "Set"       : "OMCI_MSG_TYPE_SET",
                    "Get"       : "OMCI_MSG_TYPE_GET",
		    "GetAllAlarm" : "OMCI_MSG_TYPE_GET_ALL_ALARMS",
		    "GetAllAlarmNext" : "OMCI_MSG_TYPE_GET_ALL_ALARMS_NEXT",
		    "MibUpload" : "OMCI_MSG_TYPE_MIB_UPLOAD",
		    "MibUploadNext" : "OMCI_MSG_TYPE_MIB_UPLOAD_NEXT",
	    	    "MibReset" : "OMCI_MSG_TYPE_MIB_RESET",
		    "Test" : "OMCI_MSG_TYPE_TEST",
		    "SwDownload" : "OMCI_MSG_TYPE_START_SW_DOWNLOAD",
		    "SwDownloadSec" : "OMCI_MSG_TYPE_DOWNLOAD_SECTION",
		    "SwDownloadEnd" : "OMCI_MSG_TYPE_END_SW_DOWNLOAD",
		    "ActSw" : "OMCI_MSG_TYPE_ACTIVATE_SW",
		    "CommitSw" : "OMCI_MSG_TYPE_COMMIT_SW",
		    "SyncTime" : "OMCI_MSG_TYPE_SYNCHRONIZE_TIME",
		    "Reboot" : "OMCI_MSG_TYPE_REBOOT",
		    "GetNext" : "OMCI_MSG_TYPE_GET_NEXT",
		    "GetCurData" : "OMCI_MSG_TYPE_GET_CURRENT_DATA",
                   }
    ## Constructor.
    def __init__(self, name):
        self.Name = name.strip()
        return

def mgmt_parser(key, val):
    node = nodes[0]

    if not key in mgmtNode.TOKENS:
        print 'mgmt invalid input %s %s' % (key, val)
        sys.exit(-1)

    setattr(node, key.strip(), val.strip())

    return

def process_input_file(filedir, filename):
    global MGMTNAME
    line = ''
    lineNum = 0
    nodeNum = 0

    inFile = filedir + '/' + filename

    try:
        fin = open(inFile, 'r')
    except:
        print 'Fail to open %s.' % inFile
        sys.exit(-1)

    for line in fin:
        lineNum = lineNum + 1
        #print "%d) %s" % (lineNum, line)

        line = line.strip()

        if len(line) == 0:
            continue

        ## skip #
        comment = re.search('^\#', line)
        if comment:
            continue

        #parameter = line.strip.split(":")
        key, val = map(str, line.split(":"))

        if key == KEY_ITEM:
            if nodeNum == 0:
                MGMTNAME = val.strip()
            nodeNum = nodeNum + 1
            nodes.append(mgmtNode(val))
        else:
            if nodeNum == 1:
                mgmt_parser(key, val)
            else:
                attr_parser(nodeNum - 1, key, val)
    fin.close()

    return

def actionParser(actionType):
    actionStr = ''
    actType = actionType.split(",")

    for idx in xrange(len(actType)):
        if idx >= 1:
            actionStr = actionStr + ' | '
        actionStr = actionStr + '%s' % mgmtNode.ACTION_TYPES[actType[idx]]
    return actionStr

def func_output(filedir):
    global MGMTNAME

    #outf = filedir + "/mib_%s.c" % MGMTNAME
    outf = "%s/mib_%s.c" % (DEST_DIR_FILE, MGMTNAME)

    try:
        fout = open(outf, 'w+')
    except:
        print 'Fail to open %s.' % outf
        sys.exit(-1)

    fout.write(FUNC_TITLE)

    #global variable
    fout.write('MIB_TABLE_INFO_T gMib%sTableInfo;\n' % MGMTNAME)
    fout.write('MIB_ATTR_INFO_T  gMib%sAttrInfo[MIB_TABLE_%s_ATTR_NUM];\n' % (MGMTNAME, MGMTNAME.upper()))
    fout.write('MIB_TABLE_%s_T gMib%sDefRow;\n' % (MGMTNAME.upper(), MGMTNAME))
    fout.write('MIB_TABLE_OPER_T gMib%sOper;\n' % MGMTNAME)
    fout.write('\n\n')

    #API
    #fout.write('GOS_ERROR_CODE %sAvlTreeAdd(MIB_TREE_T* pTree, %s %sId)\n' % (MGMTNAME, nodes[1].Type, MGMTNAME))
    #fout.write('{\n')
    #fout.write('	MIB_TABLE_%s_T* p%s;\n\n' % (MGMTNAME.upper(), MGMTNAME))
    #fout.write('	if(pTree == NULL) return GOS_FAIL;\n\n')
    #fout.write('	p%s = (MIB_TABLE_%s_T*)malloc(sizeof(MIB_TABLE_%s_T));\n\n' % (MGMTNAME, MGMTNAME.upper(), MGMTNAME.upper()))
    #fout.write('	if(!p%s)\n' % MGMTNAME)
    #fout.write('	{\n')
    #fout.write('	\tOMCI_LOG(OMCI_LOG_LEVEL_LOW, "malloc %s Fail");\n' % MGMTNAME)
    #fout.write('	\treturn GOS_FAIL;\n')
    #fout.write('	}\n\n')
    #fout.write('	memset(p%s, 0, sizeof(MIB_TABLE_%s_T));\n' % (MGMTNAME, MGMTNAME.upper()))
    #fout.write('	p%s->%s = %sId;\n' % (MGMTNAME, nodes[1].Name, MGMTNAME))
    #fout.write('	if(MIB_Get(MIB_TABLE_%s_INDEX, p%s, sizeof(MIB_TABLE_%s_T)) != GOS_OK)\n' % (MGMTNAME.upper(), MGMTNAME, MGMTNAME.upper()))
    #fout.write('	{\n');
    #fout.write('	\tOMCI_LOG(OMCI_LOG_LEVEL_LOW, "Get %s Fail");\n' % MGMTNAME)
    #fout.write('	\treturn GOS_FAIL;\n')
    #fout.write('	}\n\n');
    #fout.write('	if(MIB_AvlTreeNodeAdd(&pTree->root, AVL_KEY_%s, MIB_TABLE_%s_INDEX, p%s)==NULL)\n' % (MGMTNAME.upper(), MGMTNAME.upper(), MGMTNAME))
    #fout.write('	{\n');
    #fout.write('	\tOMCI_LOG(OMCI_LOG_LEVEL_LOW, "Add %s Node Fail");\n' % MGMTNAME)
    #fout.write('	\treturn GOS_FAIL;\n')
    #fout.write('	}\n\n');
    #fout.write('	return GOS_OK;\n')
    #fout.write('}\n\n')

    #API
    fout.write('GOS_ERROR_CODE %sDumpMib(void *pData)\n' % MGMTNAME)
    fout.write('{\n')
    fout.write('	MIB_TABLE_%s_T *p%s = (MIB_TABLE_%s_T*)pData;\n'% (MGMTNAME.upper(), MGMTNAME, MGMTNAME.upper()))
    fout.write('	OMCI_PRINT("%%s", "%s");\n' % MGMTNAME)
    fout.write('\n')
    for i in xrange(1, len(nodes)):
        fout.write('	OMCI_PRINT("%s: %s", p%s->%s);\n' % (nodes[i].Name, OUTSTYLE_TYPE[nodes[i].OutStyle], MGMTNAME, nodes[i].Name))
    fout.write('\n')
    fout.write('	return GOS_OK;\n')
    fout.write('}\n\n')

    #API
    #fout.write('GOS_ERROR_CODE %sConnCheck(MIB_TREE_T *pTree, MIB_TREE_CONN_T *pConn, PON_ME_ENTITY_ID entityId, int parm)\n' % MGMTNAME)
    #fout.write('{\n')
    #fout.write('	MIB_ENTRY_T *pEntry;\n')
    #fout.write('	MIB_TABLE_%s_T *p%s;\n\n' % (MGMTNAME.upper(), MGMTNAME))
    #fout.write('	OMCI_LOG(OMCI_LOG_LEVEL_LOW, "Start %s...", __FUNCTION__);\n')
    #fout.write('	pEntry = MIB_AvlTreeEntrySearch(pTree->root, AVL_KEY_%s, entityId);\n\n' % MGMTNAME.upper())
    #fout.write('	if(!pEntry){\n')
    #fout.write('	\treturn GOS_FAIL;\n')
    #fout.write('	}\n\n')
    #fout.write('	p%s = (MIB_TABLE_%s_T*)pEntry->pData;\n' % (MGMTNAME, MGMTNAME.upper()))
    #fout.write('	pConn->p%s[parm] = p%s;\n' % (MGMTNAME, MGMTNAME))
    #fout.write('	return GOS_OK;\n')
    #fout.write('}\n\n')

    #API
    #fout.write('GOS_ERROR_CODE %sConnCfg(void* pOldRow, void* pNewRow, MIB_OPERA_TYPE  operationType, MIB_ATTRS_SET attrSet, UINT32 pri)\n' % MGMTNAME)
    #fout.write('{\n')
    #fout.write('	MIB_ENTRY_T *pEntry;\n')
    #fout.write('	MIB_AVL_KEY_T key;\n')
    #fout.write('	MIB_TREE_T *pTree;\n')
    #fout.write('	MIB_TABLE_%s_T* p%s;\n\n\n' % (MGMTNAME.upper(), MGMTNAME))
    #fout.write('	switch (operationType){\n')
    #fout.write('	case MIB_ADD:\n')
    #fout.write('	{\n')
    #fout.write('	\tOMCI_LOG(OMCI_LOG_LEVEL_LOW, "%s ---- > ADD");\n' % MGMTNAME)
    #fout.write('	\tbreak;\n')
    #fout.write('	}\n')
    #fout.write('	case MIB_SET:\n')
    #fout.write('	{\n')
    #fout.write('	\tOMCI_LOG(OMCI_LOG_LEVEL_LOW, "%s ---- > SET");\n' % MGMTNAME)
    #fout.write('	\tbreak;\n')
    #fout.write('	}\n')
    #fout.write('	case MIB_DEL:\n')
    #fout.write('	{\n')
    #fout.write('	\tOMCI_LOG(OMCI_LOG_LEVEL_LOW, "%s ---- > DEL");\n' % MGMTNAME)
    #fout.write('	\tp%s = (MIB_TABLE_%s_T *)pOldRow;\n' % (MGMTNAME, MGMTNAME.upper()))
    #fout.write('	\tkey = AVL_KEY_%s;\n\n' % MGMTNAME.upper())
    #fout.write('	\tpTree = MIB_AvlTreeSearchByKey(p%s->%s,AVL_KEY_%s);\n\n' % (MGMTNAME, nodes[1].Name, MGMTNAME.upper()))
    #fout.write('	\tif(!pTree)\n')
    #fout.write('	\t{\n')
    #fout.write('	\t\tOMCI_LOG(OMCI_LOG_LEVEL_LOW, "Search %s tree faild");\n' % MGMTNAME)
    #fout.write('	\t\treturn GOS_OK;\n')
    #fout.write('	\t}\n\n')
    #fout.write('	\tpEntry = MIB_AvlTreeEntrySearch(pTree->root, key, p%s->%s);\n\n' % (MGMTNAME, nodes[1].Name))
    #fout.write('	\tif(!pEntry)\n')
    #fout.write('	\t{\n')
    #fout.write('	\t\tOMCI_LOG(OMCI_LOG_LEVEL_LOW, "Search %s entry faild");\n' % MGMTNAME)
    #fout.write('	\t\treturn GOS_OK;\n')
    #fout.write('	\t}\n')
    #fout.write('	\tOMCI_LOG(OMCI_LOG_LEVEL_LOW, "Search Entry [%x]", pEntry);\n')
    #fout.write('	\t/*remove node from tree*/\n')
    #fout.write('	\tLIST_REMOVE(pEntry,treeEntry);\n')
    #fout.write('	\t/*check connection*/\n')
    #fout.write('	\tMIB_TreeConnUpdate(pTree);\n')
    #fout.write('	\tbreak;\n')
    #fout.write('	}\n')
    #fout.write('	default:\n')
    #fout.write('	\treturn GOS_OK;\n')
    #fout.write('	\tbreak;\n')
    #fout.write('	}\n\n')
    #fout.write('	return GOS_OK;\n')
    #fout.write('}\n\n\n')

    #API: DrvCfg
    fout.write('GOS_ERROR_CODE %sDrvCfg(void* pOldRow, void* pNewRow, MIB_OPERA_TYPE operationType, MIB_ATTRS_SET attrSet, UINT32 pri)\n' % MGMTNAME)
    fout.write('{\n')
    fout.write('    OMCI_LOG(OMCI_LOG_LEVEL_LOW, "Start %s...", __FUNCTION__);\n\n')
    fout.write('    switch (operationType){\n')
    fout.write('    case MIB_ADD:\n')
    fout.write('    \tOMCI_LOG(OMCI_LOG_LEVEL_LOW,"%s --> ADD");\n' % MGMTNAME)
    fout.write('    \tbreak;\n')
    fout.write('    case MIB_SET:\n')
    fout.write('    \tOMCI_LOG(OMCI_LOG_LEVEL_LOW,"%s --> SET");\n' % MGMTNAME)
    fout.write('    \tbreak;\n')
    fout.write('    case MIB_GET:\n')
    fout.write('    \tOMCI_LOG(OMCI_LOG_LEVEL_LOW,"%s --> GET");\n' % MGMTNAME)
    fout.write('    \tbreak;\n')
    fout.write('    case MIB_DEL:\n')
    fout.write('    \tOMCI_LOG(OMCI_LOG_LEVEL_LOW,"%s --> DEL");\n' % MGMTNAME)
    fout.write('    \tbreak;\n')
    fout.write('    default:\n')
    fout.write('    \treturn GOS_FAIL;\n')
    fout.write('    \tbreak;\n')
    fout.write('    }\n\n')
    fout.write('    return GOS_OK;\n')
    fout.write('}\n')

    #init
    fout.write('GOS_ERROR_CODE mibTable_init(MIB_TABLE_INDEX tableId)\n')
    fout.write('{\n')
    fout.write('    gMib%sTableInfo.Name = "%s";\n' % (MGMTNAME, nodes[0].Name))
    fout.write('    gMib%sTableInfo.Desc = "%s";\n' % (MGMTNAME, nodes[0].Desc))
    fout.write('    gMib%sTableInfo.ClassId = (UINT32)(%s);\n' % (MGMTNAME, nodes[0].ClassId))
    fout.write('    gMib%sTableInfo.InitType = (UINT32)(%s);\n' % (MGMTNAME, INIT_TYPE[nodes[0].InitType.upper()]))
    fout.write('    gMib%sTableInfo.StdType = (UINT32)(OMCI_ME_TYPE_%s);\n' % (MGMTNAME, nodes[0].StdType.upper()))
    fout.write('    gMib%sTableInfo.ActionType = (UINT32)(%s);\n' % (MGMTNAME, actionParser(nodes[0].ActionType)))
    fout.write('    gMib%sTableInfo.pAttributes = &(gMib%sAttrInfo[0]);\n' % (MGMTNAME, MGMTNAME))
    fout.write('\n')
    fout.write('    gMib%sTableInfo.attrNum = MIB_TABLE_%s_ATTR_NUM;\n' % (MGMTNAME, MGMTNAME.upper()))
    fout.write('    gMib%sTableInfo.entrySize = sizeof(MIB_TABLE_%s_T);\n' % (MGMTNAME, MGMTNAME.upper()))
    fout.write('    gMib%sTableInfo.pDefaultRow = &gMib%sDefRow;\n' % (MGMTNAME, MGMTNAME))
    fout.write('\n')

    #Name
    for i in xrange(1, len(nodes)):
        fout.write('    gMib%sAttrInfo[MIB_TABLE_%s_%s_INDEX - MIB_TABLE_FIRST_INDEX].Name = "%s";\n'
                % (MGMTNAME, MGMTNAME.upper(), nodes[i].Name.upper(), nodes[i].Name))
    fout.write('\n')

    #Desc
    for i in xrange(1, len(nodes)):
        fout.write('    gMib%sAttrInfo[MIB_TABLE_%s_%s_INDEX - MIB_TABLE_FIRST_INDEX].Desc = "%s";\n'
                % (MGMTNAME, MGMTNAME.upper(), nodes[i].Name.upper(), nodes[i].Desc))
    fout.write('\n')

    #DataType
    for i in xrange(1, len(nodes)):
        if nodes[i].Type == 'TBL':
	    fout.write('    gMib%sAttrInfo[MIB_TABLE_%s_%s_INDEX - MIB_TABLE_FIRST_INDEX].DataType = MIB_ATTR_TYPE_TABLE;\n'
	        % (MGMTNAME, MGMTNAME.upper(), nodes[i].Name.upper()))
	else:
	    fout.write('    gMib%sAttrInfo[MIB_TABLE_%s_%s_INDEX - MIB_TABLE_FIRST_INDEX].DataType = MIB_ATTR_TYPE_%s;\n'
                % (MGMTNAME, MGMTNAME.upper(), nodes[i].Name.upper(), nodes[i].Type))
    fout.write('\n')

    #Len
    for i in xrange(1, len(nodes)):
        fout.write('    gMib%sAttrInfo[MIB_TABLE_%s_%s_INDEX - MIB_TABLE_FIRST_INDEX].Len = %s;\n'
                % (MGMTNAME, MGMTNAME.upper(), nodes[i].Name.upper(), nodes[i].Len))
    fout.write('\n')

    #IsIndex
    for i in xrange(1, len(nodes)):
        fout.write('    gMib%sAttrInfo[MIB_TABLE_%s_%s_INDEX - MIB_TABLE_FIRST_INDEX].IsIndex = %s;\n'
                % (MGMTNAME, MGMTNAME.upper(), nodes[i].Name.upper(), VALID_TYPE[nodes[i].IsIndex]))
    fout.write('\n')

    #MibSave
    for i in xrange(1, len(nodes)):
        fout.write('    gMib%sAttrInfo[MIB_TABLE_%s_%s_INDEX - MIB_TABLE_FIRST_INDEX].MibSave = %s;\n'
                % (MGMTNAME, MGMTNAME.upper(), nodes[i].Name.upper(), VALID_TYPE[nodes[i].MibSave]))
    fout.write('\n')

    #OutStyle
    for i in xrange(1, len(nodes)):
        fout.write('    gMib%sAttrInfo[MIB_TABLE_%s_%s_INDEX - MIB_TABLE_FIRST_INDEX].OutStyle = MIB_ATTR_OUT_%s;\n'
                % (MGMTNAME, MGMTNAME.upper(), nodes[i].Name.upper(), nodes[i].OutStyle.upper()))
    fout.write('\n')

    #OltAcc
    for i in xrange(1, len(nodes)):
        fout.write('    gMib%sAttrInfo[MIB_TABLE_%s_%s_INDEX - MIB_TABLE_FIRST_INDEX].OltAcc = %s;\n'
                % (MGMTNAME, MGMTNAME.upper(), nodes[i].Name.upper(), OLTACC_TYPE[nodes[i].OltAcc]))
    fout.write('\n')

    #AvcFlag
    for i in xrange(1, len(nodes)):
        fout.write('    gMib%sAttrInfo[MIB_TABLE_%s_%s_INDEX - MIB_TABLE_FIRST_INDEX].AvcFlag = %s;\n'
                % (MGMTNAME, MGMTNAME.upper(), nodes[i].Name.upper(), VALID_TYPE[nodes[i].AvcFlag]))
    fout.write('\n')

    #OptionType
    for i in xrange(1, len(nodes)):
        fout.write('    gMib%sAttrInfo[MIB_TABLE_%s_%s_INDEX - MIB_TABLE_FIRST_INDEX].OptionType = %s;\n'
                % (MGMTNAME, MGMTNAME.upper(), nodes[i].Name.upper(), OPTION_TYPE[nodes[i].OptionType]))
    fout.write('\n')

    #default value
    for i in xrange(1, len(nodes)):
        if nodes[i].Type == 'STR':
            fout.write('    strncpy(gMib%sDefRow.%s, "%s", sizeof(gMib%sDefRow.%s));\n'
                    % (MGMTNAME, nodes[i].Name, nodes[i].DefaultValue, MGMTNAME, nodes[i].Name))
	elif nodes[i].Type == 'TBL':
	    fout.write('    memset(gMib%sDefRow.%s, 0x%02x, %s);\n'
	            % (MGMTNAME, nodes[i].Name, (int)(nodes[i].DefaultValue, 16), nodes[i].Len))
        else:
	    if nodes[i].DefaultValue.find("0x") == -1:
	        fout.write('    gMib%sDefRow.%s = %d;\n'
	                % (MGMTNAME, nodes[i].Name, (int)(nodes[i].DefaultValue, 10)))
	    else:
	        fout.write('    gMib%sDefRow.%s = 0x%02x;\n'
	                % (MGMTNAME, nodes[i].Name, (int)(nodes[i].DefaultValue, 16)))
    fout.write('\n')

    fout.write('    gMib%sOper.meOperDrvCfg = %sDrvCfg;\n' % (MGMTNAME, MGMTNAME))
    #fout.write('    gMib%sOper.meOperConnCheck = %sConnCheck;\n' % (MGMTNAME, MGMTNAME))
    fout.write('    gMib%sOper.meOperConnCheck = NULL;\n' % (MGMTNAME))
    fout.write('    gMib%sOper.meOperDump = %sDumpMib;\n' % (MGMTNAME, MGMTNAME))
    #fout.write('    gMib%sOper.meOperConnCfg = %sConnCfg;\n' % (MGMTNAME, MGMTNAME))
    fout.write('    gMib%sOper.meOperConnCfg = NULL;\n' % (MGMTNAME))
    #fout.write('    gMib%sOper.meOperAvlTreeAdd = %sAvlTreeAdd;\n' % (MGMTNAME, MGMTNAME))
    fout.write('    gMib%sOper.meOperAvlTreeAdd = NULL;\n' % (MGMTNAME))
    fout.write('    gMib%sOper.meOperAlarmHandler = NULL;\n' % (MGMTNAME))
    fout.write('    gMib%sOper.meOperTestHandler = NULL;\n' % (MGMTNAME))
    fout.write('\n')

    fout.write('    MIB_TABLE_%s_INDEX = tableId;\n' % MGMTNAME.upper())
    fout.write('    MIB_InfoRegister(tableId, &gMib%sTableInfo, &gMib%sOper);\n' % (MGMTNAME, MGMTNAME))
    fout.write('    MIB_RegisterCallback(tableId, NULL, NULL);\n')
    fout.write('\n')
    fout.write('    return GOS_OK;\n')
    fout.write('}\n\n')

    fout.close()
    return

def header_output(filedir):
    global MGMTNAME

    #outf = filedir + "/mib_%s.h" % MGMTNAME
    outf = "%s/mib_%s.h" % (DEST_DIR_HEADER, MGMTNAME)

    try:
        fout = open(outf, 'w+')
    except:
        print 'Fail to open %s.' % outf
        sys.exit(-1)

    fout.write(HEADER_TITLE % (MGMTNAME.upper(), MGMTNAME.upper()))

    #attribute string
    fout.write('/* Table %s attribute for STRING type define each entry length */\n' % MGMTNAME);
    for idx in xrange(1, len(nodes)):
        if nodes[idx].Type == 'STR' or nodes[idx].Type == 'TBL':
            fout.write('#define MIB_TABLE_%s_LEN (%s)\n' % (nodes[idx].Name.upper(), nodes[idx].Len))

    fout.write('\n')

    #attribute index
    fout.write('/* Table %s attribute index */\n' % MGMTNAME)
    fout.write('#define MIB_TABLE_%s_ATTR_NUM (%d)\n' % (MGMTNAME.upper(), len(nodes) - 1))

    for idx in xrange(1, len(nodes)):
        fout.write('#define MIB_TABLE_%s_%s_INDEX ((MIB_ATTR_INDEX)%d)\n' % (MGMTNAME.upper(), nodes[idx].Name.upper(), idx))

    fout.write('\n')

    #attribute structure
    fout.write('/* Table %s attribute len, only string attrubutes have length definition */\n' % MGMTNAME)
    fout.write('typedef struct {\n')

    offset = 0
    rsv = 0
    for idx in xrange(1, len(nodes)):
        attrLen = (int)(nodes[idx].Len)
        if nodes[idx].Type == 'STR':
            fout.write('\t%-8s %s[MIB_TABLE_%s_LEN+1];\n' % ('CHAR', nodes[idx].Name, nodes[idx].Name.upper()))
            offset += 1
	elif nodes[idx].Type == 'TBL':
	    fout.write('\t%-8s %s[MIB_TABLE_%s_LEN];\n' % ('UINT8', nodes[idx].Name, nodes[idx].Name.upper()))
        else:
            if offset % 2 != 0 and 0 != ((offset + attrLen) % 4):
                rsvsize = (4 - (offset % 4))
		if attrLen < rsvsize:
	            rsvsize -= attrLen
                #fout.write('\t%-8s resv%d[%d];\n' % ('UINT8', rsv, rsvsize))
                rsv += 1
		offset += rsvsize
	    elif offset % 2 == 0 and 0 == attrLen % 4 and idx != 1 and 0 != ((offset + attrLen) % 4):
	    	rsvsize = (4 - (offset % 4))
		#fout.write('\t%-8s resv%d[%d];\n' % ('UINT8', rsv, rsvsize))
		rsv += 1
		offset += rsvsize

            fout.write('\t%-8s %s;\n' % (nodes[idx].Type, nodes[idx].Name))

        offset += attrLen

    #if offset % 4 != 0:
	#print 'total: %d' % offset
     #   rsvsize = (4 - (offset % 4))
      #  fout.write('\t%-8s resv%d[%d];\n' %('UINT8', rsv, rsvsize))
    fout.write('} __attribute__((aligned)) MIB_TABLE_%s_T;\n' % MGMTNAME.upper())
    fout.write('\n')

    fout.write('#endif /* __MIB_%s_TABLE_H__ */\n' % MGMTNAME.upper())

    fout.close()

    return

## Process the command-line argument
def main():
    f_dir   = '.'
    f_fname = ''
    # Parse input arguments
    sys.argv.pop(0) # remove mk_parser.py itself

    if len(sys.argv) == 0:
        sys.stdout.write("argv is null")
        return

    while (len(sys.argv) > 0):
        item = sys.argv.pop(0)
        if '-fd' == item:
            f_dir = sys.argv.pop(0)
        elif '-f' == item:
            f_fname = sys.argv.pop(0)
        else:
            sys.stdout.write("parameter incorrect\n")
            return

    process_input_file(f_dir, f_fname)

    func_output(f_dir)
    header_output(f_dir)
	#should be removed --seven
    #include_output(f_dir)
	#should be removed --seven
    #init_output(f_dir)
    return

# Entry point of the script
# ./pon_gen.py -fd . -f xxx.conf
if __name__ == '__main__':
    main()

