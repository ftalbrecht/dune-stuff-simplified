#ifndef DUNE_STUFF_ADAPTION_HH
#define DUNE_STUFF_ADAPTION_HH

#include <dune/stuff/restrict_prolong.hh>
#include <dune/fem/space/dgspace/localrestrictprolong.hh>
#include <dune/common/shared_ptr.hh>
#include <boost/shared_ptr.hpp>

namespace Dune {
/**
 *  \brief encapsulates the adaption handling for our DiscreteOseenFunctionWrapper
 *	Each instance produces its own RestrictProlongPair of operators for one stokesFunctionWrapper, these are in turn managed
	in a static operator set which in turn is the real argument type for the shared Dune::AdaptationManager instance.
	\note this will horribly fail if you have FunctionWrappers of same type living on different grid instances (which could be remedied by keeping a grid instance indexed map of adaption managers instead).
 *
 **/
template < class DiscreteOseenFunctionWrapperImp >
class DiscreteOseenFunctionWrapperAdaptionManager
{
	 protected:
		typedef typename DiscreteOseenFunctionWrapperImp::GridType
			GridType;

		typedef Dune::RestrictProlongDefault< typename DiscreteOseenFunctionWrapperImp::DiscretePressureFunctionType >
			RestrictProlongPressureType;
		typedef Dune::AdaptationManager< GridType, RestrictProlongPressureType >
			PressureAdaptationManagerType;

		typedef Dune::RestrictProlongDefault< typename DiscreteOseenFunctionWrapperImp::DiscreteVelocityFunctionType >
			RestrictProlongVelocityType;
		typedef Dune::AdaptationManager< GridType, RestrictProlongVelocityType >
			VelocityAdaptationManagerType;

		typedef Dune::RestrictProlongPair<RestrictProlongVelocityType&, RestrictProlongPressureType& >
			RestrictProlongPairType;
        typedef boost::shared_ptr< RestrictProlongPairType >
            RestrictProlongPairPointerType;
        typedef RestrictProlongOperatorSet<RestrictProlongPairPointerType>
			RestrictProlongOperatorSetType;

		typedef Dune::AdaptationManager< GridType, RestrictProlongOperatorSetType >
			AdaptationManagerType;

        typedef boost::shared_ptr< AdaptationManagerType >
            AdaptationManagerPointerType;

	public:
        DiscreteOseenFunctionWrapperAdaptionManager (  GridType& grid,
                                                        DiscreteOseenFunctionWrapperImp& functionWrapper )
            : grid_( grid ),
			function_wrapper_( functionWrapper),
			rpVelocity_             ( functionWrapper.discreteVelocity() ),
			rpPressure_             ( functionWrapper.discretePressure() ),
            restrictPair_ptr_( new RestrictProlongPairType( rpVelocity_, rpPressure_) )
		{
            combined_adaptManager_ptr_ = getAdaptationManager();
            assert( combined_adaptManager_ptr_ );
            restrictOperator_Set_.add( restrictPair_ptr_ );
		}

        AdaptationManagerPointerType getAdaptationManager()
        {
            static AdaptationManagerPointerType ptr( new AdaptationManagerType( grid_, restrictOperator_Set_ ) );
            return ptr;
        }

        ~DiscreteOseenFunctionWrapperAdaptionManager()
		{
            restrictOperator_Set_.remove( restrictPair_ptr_ );
		}

		void adapt()
		{
            assert( combined_adaptManager_ptr_ );
            combined_adaptManager_ptr_->adapt();
		}

        DiscreteOseenFunctionWrapperAdaptionManager( DiscreteOseenFunctionWrapperAdaptionManager& other )
            : grid_( other.grid_ ),
			function_wrapper_( other.function_wrapper_ ),
			rpVelocity_             ( function_wrapper_.discreteVelocity() ),
			rpPressure_             ( function_wrapper_.discretePressure() ),
            restrictPair_ptr_(  new RestrictProlongPairType( rpVelocity_, rpPressure_ ) )
        {}

	protected:
		GridType& grid_;
        DiscreteOseenFunctionWrapperImp& function_wrapper_;
		RestrictProlongVelocityType rpVelocity_;
		RestrictProlongPressureType rpPressure_;
        RestrictProlongPairPointerType restrictPair_ptr_;
		static RestrictProlongOperatorSetType restrictOperator_Set_;
        AdaptationManagerPointerType combined_adaptManager_ptr_;
 };
template <class T>
	typename DiscreteOseenFunctionWrapperAdaptionManager<T>::RestrictProlongOperatorSetType
        DiscreteOseenFunctionWrapperAdaptionManager<T>::restrictOperator_Set_;

} // end namespace Dune
#endif // DUNE_STUFF_ADAPTION_HH
