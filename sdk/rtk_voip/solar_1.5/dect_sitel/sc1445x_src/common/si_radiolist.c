/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file: 
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 		 scr_phbook.c
 * Purpose:		 		 
 * Created:		 		 Oct 2008
 * By:		 		 	 KF
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <si_print_api.h>

#include "scr_phbook.h"
#include "scr_double_inputbox.h"
#include "menu_functions.h"
#include "lcd_driver_api.h"
#include "sc1445x_gui_main.h"

screen_phbook_t m_ScrPhBook;
lstContactsTree_t list_of_contacts_tree;

extern phbookContact pb_contact[MAX_NUM_OF_CONTACTS];
extern int num_of_contacts;

static void scr_phbook_make_contacts_tree(void);

//create a Phone Book Screen
// It contains :
//		an info object for the title
//		a list object to load the list of contacts
//		three buttons objects
int scr_phbook_create(void *parent, obj_functions_t **cur_func)
{
 	m_ScrPhBook.parent = parent;
	m_ScrPhBook.load = scr_phbook_load;

	// create an info object for the title
	m_ScrPhBook.title  = (infoobject_t *)obj_info_create((char*)NULL, (void*)&m_ScrPhBook);
	if(m_ScrPhBook.title == NULL)
		return -1;
	m_ScrPhBook.title->left = PHBOOK_TITLE_POSITION_LEFT;
	m_ScrPhBook.title->top = PHBOOK_TITLE_POSITION_TOP;
	m_ScrPhBook.title->width = PHBOOK_TITLE_POSITION_WIDTH  ;
	m_ScrPhBook.title->height = PHBOOK_TITLE_POSITION_HEIGHT ;
 	m_ScrPhBook.title->border = 1;

	// create a list object to load the list of contacts
	m_ScrPhBook.list_of_contacts = (listobject_t *)obj_list_create(NULL, 0, (void*)&m_ScrPhBook); 
	if(m_ScrPhBook.list_of_contacts == NULL) 
		return -1;
 	m_ScrPhBook.list_of_contacts->left = PHBOOK_LIST_POSITION_LEFT;
	m_ScrPhBook.list_of_contacts->top = PHBOOK_LIST_POSITION_TOP;
	m_ScrPhBook.list_of_contacts->width = PHBOOK_LIST_POSITION_WIDTH;
	m_ScrPhBook.list_of_contacts->height = PHBOOK_LIST_POSITION_HEIGHT;
	m_ScrPhBook.list_of_contacts->selection_step = 2;
	m_ScrPhBook.list_of_contacts->border = 1;

	//create buttons
	m_ScrPhBook.buttons = (buttonsobject_t *)obj_buttons_create((void*)&m_ScrPhBook); 
	if (m_ScrPhBook.buttons==NULL) 
		return -1;

	m_ScrPhBook.buttons->left[0] = PHBOOK_BUTTON_1_POSITION_LEFT; m_ScrPhBook.buttons->left[1] = PHBOOK_BUTTON_2_POSITION_LEFT; m_ScrPhBook.buttons->left[2] = PHBOOK_BUTTON_3_POSITION_LEFT;
	m_ScrPhBook.buttons->top[0] = PHBOOK_BUTTON_1_POSITION_TOP; m_ScrPhBook.buttons->top[1] = PHBOOK_BUTTON_2_POSITION_TOP; m_ScrPhBook.buttons->top[2] = PHBOOK_BUTTON_3_POSITION_TOP;
	m_ScrPhBook.buttons->width[0] = PHBOOK_BUTTON_1_POSITION_WIDTH; m_ScrPhBook.buttons->width[1] = PHBOOK_BUTTON_2_POSITION_WIDTH; m_ScrPhBook.buttons->width[2] = PHBOOK_BUTTON_3_POSITION_WIDTH; 
	m_ScrPhBook.buttons->height[0] = PHBOOK_BUTTON_1_POSITION_HEIGHT; m_ScrPhBook.buttons->height[1] = PHBOOK_BUTTON_2_POSITION_HEIGHT; m_ScrPhBook.buttons->height[2] = PHBOOK_BUTTON_3_POSITION_HEIGHT;

	*cur_func = &m_ScrPhBook.screen_functions;
	m_ScrPhBook.pSavedFunctions = cur_func;

	scr_phbook_load();

	return 0;
}


void scr_phbook_load(void)
{
	m_gui_application.activeScreen = GUI_BOOK_SCREEN;

	lcd_clear_area(PHBOOK_TITLE_POSITION_LEFT, 0, MAX_SCREEN_WIDTH, PHBOOK_BUTTON_1_POSITION_TOP - 1);
	lcd_update(0, PHBOOK_BUTTON_1_POSITION_TOP - 1);

	// load title
  	m_ScrPhBook.title->load(m_ScrPhBook.title);
 	m_ScrPhBook.title->write(m_ScrPhBook.title, PHBOOK_TITLE);

	// load list of contacts

	// read the configuration file 
	if(phbookLoadContacts() < 0) {
		si_print(PRINT_LEVEL_ERR, "Error in phbookLoadContacts\n");
	}
	
	scr_phbook_make_contacts_tree();

	m_ScrPhBook.list_of_contacts->modify(m_ScrPhBook.list_of_contacts, &list_of_contacts_tree, list_of_contacts_tree.items, 0, 0);

	// load buttons

	m_ScrPhBook.buttons->visible[0] = 1; m_ScrPhBook.buttons->visible[1] = 1; m_ScrPhBook.buttons->visible[2]= 1; 
	m_ScrPhBook.buttons->border[0] = 1; m_ScrPhBook.buttons->border[1] = 1; m_ScrPhBook.buttons->border[2] = 1; 

	strcpy(m_ScrPhBook.buttons->label[0],"Add");
	strcpy(m_ScrPhBook.buttons->label[1],"Edit");
	strcpy(m_ScrPhBook.buttons->label[2],"Del");
	
	m_ScrPhBook.buttons->load (m_ScrPhBook.buttons);
 
	// create functions
 	memset(&m_ScrPhBook.screen_functions, 0, sizeof(m_ScrPhBook.screen_functions));
	m_ScrPhBook.screen_functions.cancel = scr_phbook_cancel;

	m_ScrPhBook.screen_functions.ok = scr_phbook_dial;;
	m_ScrPhBook.screen_functions.up = scr_phbook_up;
	m_ScrPhBook.screen_functions.down = scr_phbook_down;
	m_ScrPhBook.screen_functions.key1 = scr_phbook_key1;
	m_ScrPhBook.screen_functions.key2 = scr_phbook_key2;
	m_ScrPhBook.screen_functions.key3 = scr_phbook_key3;

	*m_ScrPhBook.pSavedFunctions = &m_ScrPhBook.screen_functions;
}


void scr_phbook_unload(void)
{
	m_ScrPhBook.title->unload (m_ScrPhBook.title);
	m_ScrPhBook.list_of_contacts->unload (m_ScrPhBook.list_of_contacts);
	m_ScrPhBook.buttons->unload (m_ScrPhBook.buttons);

	if (m_ScrPhBook.title){
		free(m_ScrPhBook.title);
		m_ScrPhBook.title=NULL;
	}
	if (m_ScrPhBook.list_of_contacts)	{
		free(m_ScrPhBook.list_of_contacts);
		m_ScrPhBook.list_of_contacts=NULL;
	}
	if (m_ScrPhBook.buttons){
		free(m_ScrPhBook.buttons);
		m_ScrPhBook.buttons=NULL;
	}

	lcd_clear_area(PHBOOK_TITLE_POSITION_LEFT, 0, MAX_SCREEN_WIDTH, PHBOOK_BUTTON_1_POSITION_TOP - 1);
	lcd_update(0, PHBOOK_BUTTON_1_POSITION_TOP - 1);
}


void scr_phbook_cancel(void)
{
	//unload current screen
	scr_phbook_unload();

	scr_mainscreen_create(NULL, m_ScrPhBook.pSavedFunctions);
}


void scr_phbook_up(void)
{
	m_ScrPhBook.list_of_contacts->movecursorup(m_ScrPhBook.list_of_contacts);
}


void scr_phbook_down(void)
{
	m_ScrPhBook.list_of_contacts->movecursordown(m_ScrPhBook.list_of_contacts);
}


void scr_phbook_key1(void)
{
	lcd_clear_area(PHBOOK_TITLE_POSITION_LEFT, 0, MAX_SCREEN_WIDTH, PHBOOK_BUTTON_1_POSITION_TOP - 1);
	lcd_update(0, PHBOOK_BUTTON_1_POSITION_TOP - 1);
 	scr_double_inputbox_create(m_ScrPhBook.load, fnAddContactPhBook, m_ScrPhBook.pSavedFunctions, "", "", "", "", GUI_LIST_ITEM_SIZE, GUI_LIST_ITEM_SIZE, NULL, NULL, "Name:", "Number:");
}


void scr_phbook_key2(void)
{
	lcd_clear_area(PHBOOK_TITLE_POSITION_LEFT, 0, MAX_SCREEN_WIDTH, PHBOOK_BUTTON_1_POSITION_TOP - 1);
	lcd_update(0, PHBOOK_BUTTON_1_POSITION_TOP - 1);

	scr_double_inputbox_create(m_ScrPhBook.load, fnEditContactPhBook, m_ScrPhBook.pSavedFunctions, m_ScrPhBook.list_of_contacts->list[(int)m_ScrPhBook.list_of_contacts->selected], m_ScrPhBook.list_of_contacts->list[m_ScrPhBook.list_of_contacts->selected + 1], "", "", GUI_LIST_ITEM_SIZE, GUI_LIST_ITEM_SIZE, NULL, NULL, "Name:", "Number:");
}


void scr_phbook_key3(void)
{
	lcd_clear_area(PHBOOK_TITLE_POSITION_LEFT, 0, MAX_SCREEN_WIDTH, PHBOOK_BUTTON_1_POSITION_TOP - 1);
	lcd_update(0, PHBOOK_BUTTON_1_POSITION_TOP - 1);

	fnDeleteContactPhBook(m_ScrPhBook.list_of_contacts->list[(int)m_ScrPhBook.list_of_contacts->selected]);
	scr_phbook_load();
}


static void scr_phbook_make_contacts_tree(void)
{
	int i;

	for(i = 0 ; (i < num_of_contacts) ; i++)
	{
		list_of_contacts_tree.lstContactsItems[2*i].item = pb_contact[i].name;
		list_of_contacts_tree.lstContactsItems[2*i].cmd_ok = NULL;
		list_of_contacts_tree.lstContactsItems[2*i].next = -1;

		list_of_contacts_tree.lstContactsItems[2*i+1].item = pb_contact[i].home_number;
		list_of_contacts_tree.lstContactsItems[2*i+1].cmd_ok = NULL;
		list_of_contacts_tree.lstContactsItems[2*i+1].next = -1;
 	}

	list_of_contacts_tree.items = num_of_contacts*2;
	list_of_contacts_tree.cmd_cancel = NULL;
	list_of_contacts_tree.parent = -1;
	list_of_contacts_tree.parent_selected_item = -1;
	list_of_contacts_tree.parent_first_item = -1;
}
void scr_phbook_dial(void)
{
  	char *pos;
  
  	if(!strcmp(list_of_contacts_tree.lstContactsItems[m_ScrPhBook.list_of_contacts->selected +1].item, "-- Empty --"))
		return;

	pos = strstr(list_of_contacts_tree.lstContactsItems [m_ScrPhBook.list_of_contacts->selected+1].item, "sip:");
 	if (pos) {
  		sc1445x_phoneapi_create_newcall(CCFSM_ATTACHED_GUI, (unsigned char*)&pos[4], 0, 0,CCFSM_CONFIG_CODEC,0);	 
	}
	else {
 		sc1445x_phoneapi_create_newcall(CCFSM_ATTACHED_GUI, (unsigned char*)list_of_contacts_tree.lstContactsItems [m_ScrPhBook.list_of_contacts->selected+1].item, 0, 0,CCFSM_CONFIG_CODEC,0);	 
	}

	scr_phbook_unload();
 	scr_mainscreen_create(NULL, m_ScrPhBook.pSavedFunctions);
}
