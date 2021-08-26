#pragma once

#include "../../aux/macros.hpp"
#include "../../config.h"
#include "../../tapes/interfaces/reverseTapeInterface.hpp"
#include "../../traits/expressionTraits.hpp"
#include "../binaryExpression.hpp"
#include "../expressionInterface.hpp"
#include "../static/staticContextActiveType.hpp"
#include "../unaryExpression.hpp"
#include "nodeInterface.hpp"

/** \copydoc codi::Namespace */
namespace codi {

  /**
   * @brief Helper class for the construction of an expression in a different context.
   *
   * Converts the leaf nodes of the expression into the static context replacements. The initialization is
   * performed via three arrays.
   *
   * Conversion and initialization is done for:
   *  - LhsExpressionInterface -> StaticContextActiveType: id = identifiers[primalValueOffset]
   *                                                       primal = primalVector[id]
   *  - ConstantExpression -> ConstantExpression: value = constantData[constantValueOffset]
   *
   * The offsets are computed from the corresponding expression traits NumberOfActiveTypeArguments and
   * NumberOfConstantTypeArguments. They are evaluated on each sub graph.
   *
   * @tparam T_Rhs  The expression type. Needs to implement ExpressionInterface.
   * @tparam T_Tape  The tape which stored the expression.
   *
   */
  template<typename T_Rhs, typename T_Tape, size_t T_primalValueOffset, size_t T_constantValueOffset, typename = void>
  struct ConstructStaticContextLogic {
    public:

      using Rhs = CODI_DD(T_Rhs, CODI_T(ExpressionInterface<double, CODI_ANY>));  ///< See ConstructStaticContextLogic.
      using Tape =
          CODI_DD(T_Tape, CODI_T(ReverseTapeInterface<double, double, CODI_ANY>));  ///< See ConstructStaticContextLogic.
      static constexpr size_t primalValueOffset = CODI_DD(T_primalValueOffset, 0);  ///< See ConstructStaticContextLogic.
      static constexpr size_t constantValueOffset =
          CODI_DD(T_constantValueOffset, 0);  ///< See ConstructStaticContextLogic.

      using Real = typename Tape::Real;                ///< See TapeTypesInterface.
      using Identifier = typename Tape::Identifier;    ///< See TapeTypesInterface.
      using PassiveReal = typename Tape::PassiveReal;  ///< Basic computation type.

      /// The resulting expression type after all nodes are replaced.
      using ResultType = CODI_DD(T_Rhs, CODI_T(ExpressionInterface<double, CODI_ANY>));

      /**
       * @brief Perform the construction
       *
       * See ConstructStaticContextLogic on how the arguments are used and which conversion are performed.
       */
      static ResultType construct(Real* primalVector, Identifier const* const identifiers,
                                  PassiveReal const* const constantData);
  };

#ifndef DOXYGEN_DISABLE

  template<typename T_Rhs, typename T_Tape, size_t T_primalValueOffset, size_t T_constantValueOffset>
  struct ConstructStaticContextLogic<T_Rhs, T_Tape, T_primalValueOffset, T_constantValueOffset,
                                     ExpressionTraits::EnableIfLhsExpression<T_Rhs>> {
    public:

      using Rhs = T_Rhs;
      using Tape = T_Tape;
      static constexpr size_t primalValueOffset = T_primalValueOffset;
      static constexpr size_t constantValueOffset = T_constantValueOffset;

      using Real = typename Tape::Real;
      using Identifier = typename Tape::Identifier;
      using PassiveReal = typename Tape::PassiveReal;

      /// Conversion from LhsExpressionInterface to StaticContextActiveType.
      using ResultType = StaticContextActiveType<Tape>;

      /// Uses primalVector[identifiers[primalValueOffset]] and identifiers[primalValueOffset].
      static ResultType construct(Real* primalVector, Identifier const* const identifiers,
                                  PassiveReal const* const constantData) {
        CODI_UNUSED(constantData);

        Identifier const identifier = identifiers[primalValueOffset];
        Real const primal = primalVector[identifier];

        return ResultType(primal, identifier);
      }
  };

  template<typename T_Rhs, typename T_Tape, size_t T_primalValueOffset, size_t T_constantValueOffset>
  struct ConstructStaticContextLogic<T_Rhs, T_Tape, T_primalValueOffset, T_constantValueOffset,
                                     ExpressionTraits::EnableIfConstantExpression<T_Rhs>> {
    public:

      using Rhs = T_Rhs;
      using Tape = T_Tape;
      static constexpr size_t primalValueOffset = T_primalValueOffset;
      static constexpr size_t constantValueOffset = T_constantValueOffset;

      using Real = typename Tape::Real;
      using Identifier = typename Tape::Identifier;
      using PassiveReal = typename Tape::PassiveReal;

      /// Conversion from ConstantExpression to ConstantExpression.
      using ResultType = ConstantExpression<typename Rhs::Real>;

      /// Uses constantData[constantValueOffset].
      static ResultType construct(Real* primalVector, Identifier const* const identifiers,
                                  PassiveReal const* const constantData) {
        CODI_UNUSED(primalVector, identifiers);

        using ConversionOperator = typename Rhs::ConversionOperator<PassiveReal>;

        return ResultType(ConversionOperator::fromDataStore(constantData[constantValueOffset]));
      }
  };

  template<typename T_Real, typename T_ArgA, typename T_ArgB, template<typename> class T_Operation, typename T_Tape,
           size_t T_primalValueOffset, size_t T_constantValueOffset>
  struct ConstructStaticContextLogic<BinaryExpression<T_Real, T_ArgA, T_ArgB, T_Operation>, T_Tape, T_primalValueOffset,
                                     T_constantValueOffset> {
    public:

      using OpReal = T_Real;
      using ArgA = T_ArgA;
      using ArgB = T_ArgB;
      template<typename T>
      using Operation = T_Operation<T>;
      using Tape = T_Tape;
      static constexpr size_t primalValueOffset = T_primalValueOffset;
      static constexpr size_t constantValueOffset = T_constantValueOffset;

      using Real = typename Tape::Real;
      using Identifier = typename Tape::Identifier;
      using PassiveReal = typename Tape::PassiveReal;

      /// Unmodified offsets for first argument.
      using ArgAConstructor = ConstructStaticContextLogic<ArgA, Tape, primalValueOffset, constantValueOffset>;
      using ArgAMod = typename ArgAConstructor::ResultType;

      /// Shift offsets by the number of occurrences in the first sub tree.
      static size_t constexpr primalValueOffsetArgB =
          primalValueOffset + ExpressionTraits::NumberOfActiveTypeArguments<ArgA>::value;
      static size_t constexpr constantValueOffsetArgB =
          constantValueOffset + ExpressionTraits::NumberOfConstantTypeArguments<ArgA>::value;
      using ArgBConstructor = ConstructStaticContextLogic<ArgB, Tape, primalValueOffsetArgB, constantValueOffsetArgB>;
      using ArgBMod = typename ArgBConstructor::ResultType;

      using ResultType = BinaryExpression<OpReal, ArgAMod, ArgBMod, Operation>;

      static ResultType construct(Real* primalVector, Identifier const* const identifiers,
                                  PassiveReal const* const constantData) {
        return ResultType(ArgAConstructor::construct(primalVector, identifiers, constantData),
                          ArgBConstructor::construct(primalVector, identifiers, constantData));
      }
  };

  template<typename T_Real, typename T_Arg, template<typename> class T_Operation, typename T_Tape,
           size_t T_primalValueOffset, size_t T_constantValueOffset>
  struct ConstructStaticContextLogic<UnaryExpression<T_Real, T_Arg, T_Operation>, T_Tape, T_primalValueOffset,
                                     T_constantValueOffset> {
    public:

      using OpReal = T_Real;
      using Arg = T_Arg;
      template<typename T>
      using Operation = T_Operation<T>;
      using Tape = T_Tape;
      static constexpr size_t primalValueOffset = T_primalValueOffset;
      static constexpr size_t constantValueOffset = T_constantValueOffset;

      using Real = typename Tape::Real;
      using Identifier = typename Tape::Identifier;
      using PassiveReal = typename Tape::PassiveReal;

      /// Unmodified offsets since there is just one sub tree.
      using ArgConstructor = ConstructStaticContextLogic<Arg, Tape, primalValueOffset, constantValueOffset>;
      using ArgMod = typename ArgConstructor::ResultType;

      using ResultType = UnaryExpression<OpReal, ArgMod, Operation>;

      static ResultType construct(Real* primalVector, Identifier const* const identifiers,
                                  PassiveReal const* const constantData) {
        return ResultType(ArgConstructor::construct(primalVector, identifiers, constantData));
      }
  };
#endif
}
