/*
 * date.x - Specification of remote date, time, date and time service.
 */

/*
 * Define 1 procedure :
 * date_1() accepts a long and returns a string. 
 */

program DATE_PROG {
    version DATE_VERS {
	string	DATE(long) = 1;     /* procedure number = 1 */ 
    string	SYSTEM_STATS(long) = 2;     /* procedure number = 1 */ 
    } = 1;			                /* version number = 1     */
} = 0x31234567;			   /* program number        */

