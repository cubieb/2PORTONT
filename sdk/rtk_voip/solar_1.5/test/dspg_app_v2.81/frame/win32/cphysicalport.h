/*

  PhysicalPort Server class

*/



#if	!defined( _CPHYSICALPORT_INC )
#define	_CPHYSICALPORT_INC

#define MAXBLOCK        70

#define  CPHYSER_115200			115200



// Flow control flags

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04

// ascii definitions

#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

typedef PVOID	PDATA;		// Pointer to user data
typedef DCB		PORTCFG;		// Data structure that contains the port parameters
typedef DCB *	PPORTCFG;


typedef  void  (* PFN_RX_DATA_RECEIVE) ( void * pv_ApplRef );

class CPhysicalPort
{
	public:
		CPhysicalPort()
		{
			strcpy ( m_chPort, "COM2" );
			m_port= m_chPort;
			m_fPortConnected = FALSE;
		}

		void	setCMBSSet( PORTCFG portCfg );
		virtual BOOL        openPort( LPSTR lpstr_Port );
		virtual	BOOL		open();
		virtual	BOOL		opened(){return m_fPortConnected;}
		virtual	BOOL		close();
		virtual	BOOL		setConfig(PORTCFG portCfg);
		//virtual	BOOL		setConfig (P_DCOM_SERPARAM_MSSG serParam);
		virtual	PORTCFG	getConfig();
		virtual	BYTE		getModemState(){return m_MsrBits;}
		virtual	void		setModemState(DWORD dwModemState);
					void		setMSRBits(BYTE bModemState);
		virtual 	BOOL		setMCR(BYTE bModemState);
		virtual	void		setRTS(BYTE bRTS);

		virtual	int		read( LPSTR lpszBlock, int nMaxLength );
		virtual	BOOL		write( LPSTR lpByte , DWORD dwBytesToWrite);
		virtual	DWORD		CommWatchProc(  );
		static	void		ThreadEntry( PVOID pReference )
		{
			((CPhysicalPort *)pReference)->CommWatchProc();
		}


		static void CALLBACK TimeProc(	UINT uID,
													UINT uMsg,
													DWORD dwUser,
													DWORD dw1,
													DWORD dw2
												)
		{
			CPhysicalPort *physicalPort = (CPhysicalPort *) dwUser;
			physicalPort->TimerProc();
		}
		virtual	void		TimerProc();
                void		setRXNotifier( PFN_RX_DATA_RECEIVE pfn )
				{
					m_pfnDataReceived = pfn;
				}


	private:
		char            m_chPort[10];
		char*				m_port;
		HANDLE			m_hPort,
							m_hWatchThread;
		PORTCFG			m_PortCfg;			// contains all configuration params for the port
		COMMTIMEOUTS	m_ComTimeOuts;

		OVERLAPPED		m_osWrite,
							m_osRead ;
		volatile DWORD	m_dwThreadID ;
		BOOL				m_fDisplayErrors,
							m_fPortConnected;
		BYTE				m_bFlowCtrl,
							m_MsrBits;
		PFN_RX_DATA_RECEIVE m_pfnDataReceived;

};	// CPhysicalPort

#endif // _CPHYSICALPORT_INC
