#include "qgmPlMthMatcherFilter.hpp"
#include "pdTrace.hpp"
#include "qgmTrace.hpp"

using namespace bson;

namespace engine
{
   qgmPlMthMatcherFilter::qgmPlMthMatcherFilter( const qgmOPFieldVec &selector,
                                                INT64 numSkip,
                                                INT64 numReturn,
                                                const qgmField &alias )
   :_qgmPlFilter( selector, NULL, numSkip, numReturn, alias )
   {
   }

   INT32 qgmPlMthMatcherFilter::loadPattern( bson::BSONObj matcher )
   {
      return _mthMatcher.loadPattern( matcher );
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__QGMPLMTHMATCHERFILTER__FETCHNEXT, "qgmPlMthMatcherFilter::_fetchNext" )
   INT32 qgmPlMthMatcherFilter::_fetchNext( qgmFetchOut & next )
   {
      PD_TRACE_ENTRY( SDB__QGMPLMTHMATCHERFILTER__FETCHNEXT ) ;
      INT32 rc = SDB_OK ;
      qgmFetchOut fetch ;
      _qgmPlan *in = input( 0 ) ;

      if ( 0 <= _return && _return <= _currentReturn )
      {
         close() ;
         rc = SDB_DMS_EOC ;
         goto error ;
      }

      while ( TRUE )
      {
         rc = in->fetchNext( fetch ) ;
         if ( SDB_OK != rc && SDB_DMS_EOC != rc )
         {
            goto error ;
         }
         else if ( SDB_DMS_EOC == rc )
         {
            break ;
         }
         else
         {
         }

         BOOLEAN r = FALSE ;
         rc = _mthMatcher.matches( fetch.obj, r );
         if ( rc != SDB_OK )
         {
            goto error;
         }
         else if ( !r )
         {
            continue;
         }
         else
         {
         }

         if ( 0 < _skip && ++_currentSkip <= _skip )
         {
            continue ;
         }

         if ( !_selector.empty() )
         {
            rc = _selector.select( fetch,
                                   next.obj ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }
         }
         else
         {
            next.obj = fetch.mergedObj() ;
         }

         if ( !_merge )
         {
            next.alias = _alias.empty()?
                         fetch.alias : _alias ;
         }

         ++_currentReturn ;
         break ;
      }
   done:
      PD_TRACE_EXITRC( SDB__QGMPLMTHMATCHERFILTER__FETCHNEXT, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}
