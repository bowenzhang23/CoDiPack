#pragma once

#include "../../aux/macros.hpp"
#include "../../config.h"
#include "../data/position.hpp"
#include "positionalEvaluationTapeInterface.hpp"

/** \copydoc codi::Namespace */
namespace codi {

  template<typename _Real, typename _Identifier, typename _Position>
  struct PrimalEvaluationTapeInterface : public virtual PositionalEvaluationTapeInterface<_Position> {
    public:

      using Real = CODI_DECLARE_DEFAULT(_Real, double);
      using Identifier = CODI_DECLARE_DEFAULT(_Identifier, int);
      using Position = CODI_DECLARE_DEFAULT(_Position, EmptyPosition);

      /*******************************************************************************
       * Section: Start of interface definition
       *
       */

      static bool constexpr HasPrimalValues = CODI_UNDEFINED_VALUE;
      static bool constexpr RequiresPrimalRestore = CODI_UNDEFINED_VALUE;

      void evaluatePrimal(Position const& start, Position const& end);
      void evaluatePrimal();

      void setPrimal(Identifier const& identifier, Real const& gradient);
      Real const& getPrimal(Identifier const& identifier) const;

      Real& primal(Identifier const& identifier);
      Real const& primal(Identifier const& identifier) const;


  };
}
