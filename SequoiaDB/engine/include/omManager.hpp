/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = omManager.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OM_MANAGER_HPP__
#define OM_MANAGER_HPP__

#include "omDef.hpp"
#include "ossLatch.hpp"
#include "pmdRestSession.hpp"
#include "pmdObjBase.hpp"
#include "restAdaptor.hpp"
#include "sdbInterface.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "dmsCB.hpp"
#include "rtnCB.hpp"
#include "netRouteAgent.hpp"
#include "pmdRemoteSession.hpp"
#include "omMsgEventHandler.hpp"

#include <vector>
#include <string>
#include <map>

using namespace std ;
using namespace bson ;

namespace engine
{
   class _pmdEDUCB ;

   #define OM_TASK_STATUS_IDLE            0
   #define OM_TASK_STATUS_DOING           1
   #define OM_TASK_STATUS_ERROR_ROLLBACK  2
   #define OM_TASK_STATUS_ERROR_FINISH    3
   #define OM_TASK_STATUS_FINISH          4


   /*
      omAgentInfo define
   */
   struct omAgentInfo
   {
      UINT64   _id ;
      string   _host ;
      string   _service ;
   } ;

   class omHostVersion ;
   class omTaskManager ;
   /*
      _omManager define
   */
   class _omManager : public _pmdObjBase, public _IControlBlock
   {
      DECLARE_OBJ_MSG_MAP()

      typedef map< UINT64, omAgentInfo* >    MAP_ID2HOSTPTR ;
      typedef MAP_ID2HOSTPTR::iterator       MAP_ID2HOSTPTR_IT ;

      typedef map< string, omAgentInfo >     MAP_HOST2ID ;
      typedef MAP_HOST2ID::iterator          MAP_HOST2ID_IT ;

      public:
         _omManager() ;
         virtual ~_omManager() ;

         virtual SDB_CB_TYPE cbType() const { return SDB_CB_OMSVC ; }
         virtual const CHAR* cbName() const { return "OMSVC" ; }

         virtual INT32  init () ;
         virtual INT32  active () ;
         virtual INT32  deactive () ;
         virtual INT32  fini () ;
         virtual void   onConfigChange() {}

         virtual void   attachCB( _pmdEDUCB *cb ) ;
         virtual void   detachCB( _pmdEDUCB *cb ) ;

         UINT32      setTimer( UINT32 milliSec ) ;
         void        killTimer( UINT32 timerID ) ;

         CHAR*       allocFixBuf() ;
         INT32       getFixBufSize() const { return _fixBufSize ; }
         void        releaseFixBuf( CHAR *pBuff ) ;

         netRouteAgent* getRouteAgent() ;
         MsgRouteID     updateAgentInfo( const string &host,
                                         const string &service ) ;
         MsgRouteID     getAgentIDByHost( const string &host ) ;
         INT32          getHostInfoByID( MsgRouteID routeID,
                                         string &host,
                                         string &service ) ;
         void           delAgent( MsgRouteID routeID ) ;
         void           delAgent( const string &host ) ;

         restSessionInfo*  attachSessionInfo( const string &id ) ;
         void              detachSessionInfo( restSessionInfo *pSessionInfo ) ;

         restSessionInfo*  newSessionInfo( const string &userName,
                                           UINT32 localIP ) ;
         void              releaseSessionInfo ( const string &sessionID ) ;

         restAdaptor*      getRestAdptor() { return &_restAdptor ; }
         pmdRemoteSessionMgr* getRSManager() { return &_rsManager ; }

         INT32             authenticate( BSONObj &obj, _pmdEDUCB *cb ) ;
         INT32             authUpdatePasswd( string user, string oldPasswd,
                                             string newPasswd, pmdEDUCB *cb ) ;

         string            getLocalAgentPort() ;

         INT32             refreshVersions() ;
         void              updateClusterVersion( string cluster ) ;
         void              removeClusterVersion( string cluster ) ;

         omTaskManager     *getTaskManager() ;

      protected:
         virtual void      onTimer ( UINT64 timerID, UINT32 interval ) ;

         MsgRouteID        _incNodeID() ;
         string            _makeID( restSessionInfo *pSessionInfo ) ;

         void              _add2UserMap( const string &user,
                                         restSessionInfo *pSessionInfo ) ;
         void              _delFromUserMap( const string &user,
                                            restSessionInfo *pSessionInfo ) ;

         void              _invalidSessionInfo( restSessionInfo *pSessionInfo ) ;
         void              _checkSession( UINT32 interval ) ;

         INT32             _initOmTables() ;

         INT32             _createJobs() ;

         INT32             _createCollectionIndex ( const CHAR *pCollection,
                                                    const CHAR *pIndex,
                                                    pmdEDUCB *cb ) ;

         INT32             _createCollection ( const CHAR *pCollection,
                                               pmdEDUCB *cb ) ;
         void              _readAgentPort() ;

         INT32             _restoreTask() ;

         INT32             _onAgentQueryTaskReq( NET_HANDLE handle, 
                                                 MsgHeader *pMsg ) ;
         BOOLEAN           _isCommand( const CHAR *pCheckName ) ;
         void              _sendRes2Agent( NET_HANDLE handle, 
                                           MsgHeader *pSrcMsg, 
                                           INT32 flag, BSONObj response ) ;


      protected:

      private:
         vector< CHAR* >                        _vecFixBuf ;
         const INT32                            _fixBufSize ;

         map<string, restSessionInfo*>          _mapSessions ;
         map<string, vector<restSessionInfo*> > _mapUser2Sessions ;
         UINT32                                 _sequence ;
         UINT32                                 _checkSessionTimer ;

         MAP_ID2HOSTPTR                         _mapID2Host ;
         MAP_HOST2ID                            _mapHost2ID ;
         MsgRouteID                             _hwRouteID ;

         ossSpinSLatch                          _omLatch ;
         ossEvent                               _attachEvent ;

         restAdaptor                            _restAdptor ;
         pmdRemoteSessionMgr                    _rsManager ;

         omMsgHandler                           _msgHandler ;
         omTimerHandler                         _timerHandler ;
         netRouteAgent                          _netAgent ;
         MsgRouteID                             _myNodeID ;

         INT32                                  _maxRestBodySize ;
         INT32                                  _restTimeout ;

         pmdKRCB*                               _pKrcb ;
         SDB_DMSCB*                             _pDmsCB ;
         SDB_RTNCB*                             _pRtnCB ;

         string                                 _wwwRootPath ;

         string                                 _localAgentPort ;
         omHostVersion                          *_hostVersion ;

         omTaskManager                          *_taskManager ;
   } ;

   typedef _omManager omManager ;
   /*
      get the global om manager object point
   */
   omManager *sdbGetOMManager() ;

}

#endif // OM_MANAGER_HPP__

