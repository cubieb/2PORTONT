/*****************************************************
 ecmh - Easy Cast du Multi Hub - Common Functions
******************************************************
 $Author: masonyu $
 $Id: common.h,v 1.1 2011/02/16 11:32:12 masonyu Exp $
 $Date: 2011/02/16 11:32:12 $
*****************************************************/

void dolog(int level, const char *fmt, ...);
int huprunning(void);
void savepid(void);
void cleanpid(int i);
