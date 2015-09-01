#include <Framework/Script/Engine.hpp>

#include <Framework/Script/Metatable.hpp>
#include <Framework/Script/Object.hpp>
#include <Framework/Script/Sandbox.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

TEST(UnitScript_Utils, ShouldDemangleClassName)
{
	EXPECT_EQ("int", Script::Utils::DemangleClassName< int >());
	EXPECT_EQ("bool", Script::Utils::DemangleClassName< bool >());
}

TEST(UnitScript_Utils, ShouldDemangleClassNameWithNamespace)
{
	using namespace Script;
	EXPECT_EQ("ScriptEngine", (Script::Utils::DemangleClassName< Engine >()));
	EXPECT_EQ("ScriptEngine", (Script::Utils::DemangleClassName< Script::Engine >()));
}

class UnitScript : public testing::Test
{
protected:
	inline static size_t CreatedObjects = 0;

	void TearDown() override
	{
		ShouldFreeCollectedGabage();
		ASSERT_TRUE(script.IsStackTop());
	}

	void ShouldFreeCollectedGabage()
	{
		script.CollectGarbage();
		ASSERT_EQ(CreatedObjects, size_t{ 0 });
	}

protected:
	Script::Engine script;
};

class UnitScript_Patch : public UnitScript
{
};

TEST_F(UnitScript_Patch, ShouldCheckContinueKeyword)
{
	constexpr auto code = R"(
		local Variable = 1;
		for i = 1, 3 do
			if (i == 2) then
				continue;
			end
			Variable = Variable + 1;
		end
		return Variable;
	)";

	ASSERT_EQ(script.Execute(code).Get< int32_t >(), 3);
}

TEST_F(UnitScript_Patch, ShouldCheckPlusPlusOperator)
{
	constexpr auto code = R"(
		local Variable = 1;
		Variable++;
		return Variable;
	)";

	ASSERT_EQ(script.Execute(code).Get< int32_t >(), 2);
}

TEST_F(UnitScript_Patch, ShouldCheckPlusMinusEqualOperator)
{
	constexpr auto code = R"(
		local Variable = 1;
		Variable += 3;
		Variable -= 1;
		return Variable;
	)";

	ASSERT_EQ(script.Execute(code).Get< int32_t >(), 3);
}

class UnitScript_Execute : public UnitScript
{
protected:
	inline static const std::string ReturnNil = R"(return nil)";
	inline static const std::string ReturnBoolean = R"(return true)";
	inline static const std::string ReturnNumber = R"(return 1 + 2)";
	inline static const std::string ReturnFloatingNumber = R"(return 3.14)";
	inline static const std::string ReturnString = R"(return "FooBar")";
	inline static const std::string ReturnContainer = R"(return {3, 2, 3, 1})";
	inline static const std::string ReturnContainerMap = R"(return { [1] = 2, [2] = 3, [3] = 1 })";
	inline static const std::string ReturnFunction = R"(return function(value) return "Foo"..value; end)";
};

TEST_F(UnitScript_Execute, ShouldReturnBasicType)
{
	EXPECT_EQ(script.Execute(ReturnNil).GetType(), Script::VariableType::Nil);
	EXPECT_EQ(script.Execute(ReturnBoolean).GetType(), Script::VariableType::Boolean);
	EXPECT_EQ(script.Execute(ReturnNumber).GetType(), Script::VariableType::Number);
	EXPECT_EQ(script.Execute(ReturnFloatingNumber).GetType(), Script::VariableType::Number);
	EXPECT_EQ(script.Execute(ReturnString).GetType(), Script::VariableType::String);
	EXPECT_EQ(script.Execute(ReturnContainer).GetType(), Script::VariableType::Table);
	EXPECT_EQ(script.Execute(ReturnContainerMap).GetType(), Script::VariableType::Table);
	EXPECT_EQ(script.Execute(ReturnFunction).GetType(), Script::VariableType::Function);
}

TEST_F(UnitScript_Execute, ShouldReturnBasicTypeValue)
{
	EXPECT_EQ(script.Execute(ReturnBoolean).Get< bool >(), true);
	EXPECT_EQ(script.Execute(ReturnNumber).Get< uint8_t >(), uint8_t{ 3 });
	EXPECT_EQ(script.Execute(ReturnNumber).Get< int8_t >(), int8_t{ 3 });
	EXPECT_EQ(script.Execute(ReturnNumber).Get< int16_t >(), int16_t{ 3 });
	EXPECT_EQ(script.Execute(ReturnNumber).Get< uint16_t >(), uint16_t{ 3 });
	EXPECT_EQ(script.Execute(ReturnNumber).Get< int32_t >(), int32_t{ 3 });
	EXPECT_EQ(script.Execute(ReturnNumber).Get< uint32_t >(), uint32_t{ 3 });
	EXPECT_EQ(script.Execute(ReturnNumber).Get< int64_t >(), int64_t{ 3 });
	EXPECT_EQ(script.Execute(ReturnNumber).Get< uint64_t >(), uint64_t{ 3 });
	EXPECT_NEAR(script.Execute(ReturnFloatingNumber).Get< float >(), float{ 3.14 }, 0.000001);
	EXPECT_NEAR(script.Execute(ReturnFloatingNumber).Get< double >(), double{ 3.14 }, 0.000001);
	EXPECT_EQ(script.Execute(ReturnString).Get< std::string >(), "FooBar");
}

TEST_F(UnitScript_Execute, ShouldReturnOptionalBasicTypeValue)
{
	EXPECT_EQ(script.Execute(ReturnBoolean).Get< std::optional< bool > >().value_or(false), true);
	EXPECT_EQ(script.Execute(ReturnNumber).Get< std::optional< int32_t > >().value_or(0), 3);
	EXPECT_NEAR(script.Execute(ReturnFloatingNumber).Get< std::optional< float > >().value_or(0), 3.14, 0.000001);
	EXPECT_NEAR(script.Execute(ReturnFloatingNumber).Get< std::optional< double > >().value_or(0), 3.14, 0.000001);
	EXPECT_EQ(script.Execute(ReturnString).Get< std::optional< std::string > >().value_or(""), "FooBar");
}

TEST_F(UnitScript_Execute, ShouldReturnVariantTypeValue)
{
	using VariantType = std::variant< std::monostate, bool, int32_t, std::string >;

	EXPECT_EQ(std::get< std::monostate >(script.Execute(ReturnNil).Get< VariantType >()), std::monostate{});
	EXPECT_EQ(std::get< bool >(script.Execute(ReturnBoolean).Get< VariantType >()), true);
	EXPECT_EQ(std::get< int32_t >(script.Execute(ReturnNumber).Get< VariantType >()), 3);
	EXPECT_EQ(std::get< std::string >(script.Execute(ReturnString).Get< VariantType >()), "FooBar");
}

TEST_F(UnitScript_Execute, ShouldReturnContainerSequencedValue)
{
	EXPECT_THAT(script.Execute(ReturnContainer).Get< std::deque< int32_t > >(), ContainerEq(std::deque{ 3, 2, 3, 1 }));
	EXPECT_THAT(script.Execute(ReturnContainer).Get< std::list< int32_t > >(), ContainerEq(std::list{ 3, 2, 3, 1 }));
	EXPECT_THAT(script.Execute(ReturnContainer).Get< std::vector< int32_t > >(), ContainerEq(std::vector{ 3, 2, 3, 1 }));
}

TEST_F(UnitScript_Execute, ShouldReturnContainerAssociativedValue)
{
	EXPECT_THAT(script.Execute(ReturnContainer).Get< std::set< int32_t > >(), ContainerEq(std::set{ 1, 2, 3 }));
	EXPECT_THAT(script.Execute(ReturnContainer).Get< std::multiset< int32_t > >(), ContainerEq(std::multiset{ 1, 2, 3, 3 }));
	EXPECT_THAT(script.Execute(ReturnContainer).Get< std::unordered_set< int32_t > >(), ContainerEq(std::unordered_set{ 1, 2, 3 }));
}

TEST_F(UnitScript_Execute, ShouldReturnMapContainerAssociativedValue)
{
	EXPECT_THAT(
		(script.Execute(ReturnContainer).Get< std::map< int32_t, int32_t > >()),
		(ElementsAre(Pair(1, 3), Pair(2, 2), Pair(3, 3), Pair(4, 1))));
	EXPECT_THAT(
		(script.Execute(ReturnContainer).Get< std::multimap< int32_t, int32_t > >()),
		(ElementsAre(Pair(1, 3), Pair(2, 2), Pair(3, 3), Pair(4, 1))));
	EXPECT_THAT(
		(script.Execute(ReturnContainer).Get< std::unordered_map< int32_t, int32_t > >()),
		(UnorderedElementsAre(Pair(1, 3), Pair(2, 2), Pair(3, 3), Pair(4, 1))));
	EXPECT_THAT(
		(script.Execute(ReturnContainer).Get< std::unordered_multimap< int32_t, int32_t > >()),
		(UnorderedElementsAre(Pair(1, 3), Pair(2, 2), Pair(3, 3), Pair(4, 1))));

	EXPECT_THAT(
		(script.Execute(ReturnContainerMap).Get< std::map< int32_t, int32_t > >()),
		(ElementsAre(Pair(1, 2), Pair(2, 3), Pair(3, 1))));
	EXPECT_THAT(
		(script.Execute(ReturnContainerMap).Get< std::multimap< int32_t, int32_t > >()),
		(ElementsAre(Pair(1, 2), Pair(2, 3), Pair(3, 1))));
	EXPECT_THAT(
		(script.Execute(ReturnContainerMap).Get< std::unordered_map< int32_t, int32_t > >()),
		(UnorderedElementsAre(Pair(1, 2), Pair(2, 3), Pair(3, 1))));
	EXPECT_THAT(
		(script.Execute(ReturnContainerMap).Get< std::unordered_multimap< int32_t, int32_t > >()),
		(UnorderedElementsAre(Pair(1, 2), Pair(2, 3), Pair(3, 1))));
}

TEST_F(UnitScript_Execute, ShouldReturnFunctionValue)
{
	EXPECT_THAT(script.Execute(ReturnFunction)("Bar").Get< std::string >(), "FooBar");
}

TEST_F(UnitScript_Execute, ShouldReturnEnumValue)
{
	static const std::string ReturnEnum = R"(return 1)";
	enum class Numbers : int32_t {
		First = 1,
	};

	EXPECT_THAT(script.Execute(ReturnEnum).Get< Numbers >(), Numbers::First);
}

class UnitScript_Global : public UnitScript
{
protected:
	inline static const std::string VariableNil = R"(VariableNil)";
	inline static const std::string VariableBoolean = R"(VariableBoolean)";
	inline static const std::string VariableNumber = R"(VariableNumber)";
	inline static const std::string VariableFloatingNumber = R"(VariableFloatingNumber)";
	inline static const std::string VariableString = R"(VariableString)";
	inline static const std::string VariableContainer = R"(VariableContainer)";
	inline static const std::string VariableMapContainer = R"(VariableMapContainer)";
};

TEST_F(UnitScript_Global, ShouldAssignBasicType)
{
	script.SetGlobal(VariableNil, nullptr);
	script.SetGlobal(VariableBoolean, true);
	script.SetGlobal(VariableNumber, int32_t{ 123 });
	script.SetGlobal(VariableFloatingNumber, 3.14);
	script.SetGlobal(VariableString, std::string{ "FooBar" });

	EXPECT_EQ(script[ VariableNil ].GetType(), Script::VariableType::Nil);
	EXPECT_EQ(script[ VariableBoolean ].Get< bool >(), true);
	EXPECT_EQ(script[ VariableNumber ].Get< int32_t >(), 123);
	EXPECT_NEAR(script[ VariableFloatingNumber ].Get< double >(), double{ 3.14 }, 0.000001);
	EXPECT_EQ(script[ VariableString ].Get< std::string >(), "FooBar");
}

TEST_F(UnitScript_Global, ShouldAssignOptionalValue)
{
	script.SetGlobal(VariableNil, std::optional< std::string >{});
	script.SetGlobal(VariableString, std::optional< std::string >{ "FooBar" });

	EXPECT_EQ(script[ VariableNil ].GetType(), Script::VariableType::Nil);
	EXPECT_EQ(script[ VariableString ].GetType(), Script::VariableType::String);
	EXPECT_EQ(script[ VariableString ].Get< std::optional< std::string > >().value(), "FooBar");
}

TEST_F(UnitScript_Global, ShouldAssignVariantValue)
{
	using VariantType = std::variant< std::monostate, int32_t, std::string >;

	script.SetGlobal(VariableNil, VariantType{});
	script.SetGlobal(VariableNumber, VariantType{ 123 });
	script.SetGlobal(VariableString, VariantType{ "FooBar" });

	ASSERT_EQ(script[ VariableNil ].GetType(), Script::VariableType::Nil);
	ASSERT_EQ(script[ VariableNumber ].GetType(), Script::VariableType::Number);
	ASSERT_EQ(script[ VariableString ].GetType(), Script::VariableType::String);
	EXPECT_EQ(std::get< std::monostate >(script[ VariableNil ].Get< VariantType >()), std::monostate{});
	EXPECT_EQ(std::get< int32_t >(script[ VariableNumber ].Get< VariantType >()), 123);
	EXPECT_EQ(std::get< std::string >(script[ VariableString ].Get< VariantType >()), "FooBar");
}

TEST_F(UnitScript_Global, ShouldAssignContainerSequencedValue)
{
	script.SetGlobal(VariableContainer, std::deque< int32_t >{ 3, 2, 3, 1 });
	EXPECT_THAT(script[ VariableContainer ].Get< std::deque< int32_t > >(), ContainerEq(std::deque{ 3, 2, 3, 1 }));

	script.SetGlobal(VariableContainer, std::list< int32_t >{ 3, 2, 3, 1 });
	EXPECT_THAT(script[ VariableContainer ].Get< std::list< int32_t > >(), ContainerEq(std::list{ 3, 2, 3, 1 }));

	script.SetGlobal(VariableContainer, std::vector< int32_t >{ 3, 2, 3, 1 });
	EXPECT_THAT(script[ VariableContainer ].Get< std::vector< int32_t > >(), ContainerEq(std::vector{ 3, 2, 3, 1 }));
}

TEST_F(UnitScript_Global, ShouldAssignContainerAssociativedValue)
{
	script.SetGlobal(VariableContainer, std::set< int32_t >{ 3, 2, 1 });
	EXPECT_THAT(script[ VariableContainer ].Get< std::set< int32_t > >(), ContainerEq(std::set{ 3, 2, 1 }));

	script.SetGlobal(VariableContainer, std::multiset< int32_t >{ 3, 2, 3, 1 });
	EXPECT_THAT(script[ VariableContainer ].Get< std::multiset< int32_t > >(), ContainerEq(std::multiset{ 3, 2, 3, 1 }));

	script.SetGlobal(VariableContainer, std::unordered_set< int32_t >{ 3, 2, 1 });
	EXPECT_THAT(script[ VariableContainer ].Get< std::unordered_set< int32_t > >(), ContainerEq(std::unordered_set{ 3, 2, 1 }));
}

TEST_F(UnitScript_Global, ShouldAssignMapContainerAssociativedValue)
{
	script.SetGlobal(VariableMapContainer, std::map< int32_t, int32_t >{ { 2, 5 }, { 7, 8 } });
	EXPECT_THAT(
		(script[ VariableMapContainer ].Get< std::map< int32_t, int32_t > >()),
		(ElementsAre(Pair(2, 5), Pair(7, 8))));

	script.SetGlobal(VariableMapContainer, std::multimap< int32_t, int32_t >{ { 2, 5 }, { 7, 8 } });
	EXPECT_THAT(
		(script[ VariableMapContainer ].Get< std::multimap< int32_t, int32_t > >()),
		(ElementsAre(Pair(2, 5), Pair(7, 8))));

	script.SetGlobal(VariableMapContainer, std::unordered_map< int32_t, int32_t >{ { 2, 5 }, { 7, 8 } });
	EXPECT_THAT(
		(script[ VariableMapContainer ].Get< std::unordered_map< int32_t, int32_t > >()),
		(UnorderedElementsAre(Pair(2, 5), Pair(7, 8))));

	script.SetGlobal(VariableMapContainer, std::unordered_multimap< int32_t, int32_t >{ { 2, 5 }, { 7, 8 } });
	EXPECT_THAT(
		(script[ VariableMapContainer ].Get< std::unordered_multimap< int32_t, int32_t > >()),
		(UnorderedElementsAre(Pair(2, 5), Pair(7, 8))));
}

TEST_F(UnitScript_Global, ShouldAssignEnumValue)
{
	enum class Numbers : int32_t {
		First = 1,
	};

	script.SetGlobal("Variable", Numbers::First);
	EXPECT_THAT(script[ "Variable" ].Get< Numbers >(), Numbers::First);
}

TEST_F(UnitScript_Global, ShouldAssignVectorOfPairsValue)
{
	using VariantType = std::pair< int32_t, std::string >;

	script.SetGlobal("Variable", VariantType{ 123, "foo" });
	EXPECT_EQ((script[ "Variable" ].Get< VariantType >()), (VariantType{ 123, "foo" }));
}

class UnitScript_GlobalFunction : public UnitScript
{
protected:
	inline static const std::string VariableFunction = R"(VariableFunction)";

	inline static auto StaticFunction(const int32_t value) -> std::string
	{
		return "FooBar_" + std::to_string(value);
	}

	inline static const std::function< std::string(int32_t) > WrapperFunction = {
		[](const int32_t value) {
			return "FooBar_" + std::to_string(value);
		},
	};
};

TEST_F(UnitScript_GlobalFunction, ShouldAssignStatic)
{
	script.SetGlobal(VariableFunction, StaticFunction);

	ASSERT_EQ(script.GetGlobal(VariableFunction).GetType(), Script::VariableType::Function);
	EXPECT_THAT(script.GetGlobal(VariableFunction)(123).Get< std::string >(), "FooBar_123");
}

TEST_F(UnitScript_GlobalFunction, ShouldAssignWrapper)
{
	script.SetGlobal(VariableFunction, WrapperFunction);

	ASSERT_EQ(script.GetGlobal(VariableFunction).GetType(), Script::VariableType::Function);
	EXPECT_THAT(script.GetGlobal(VariableFunction)(123).Get< std::string >(), "FooBar_123");
}

TEST_F(UnitScript_GlobalFunction, ShouldReplacePrint)
{
	std::vector< std::string > printResult = {};
	const Script::Reference tostring = script[ "tostring" ];
	const auto print = std::function{
		[ &printResult, tostring ](lua_State* L) {
			const int32_t argumentsCount = lua_gettop(L);
			for (int32_t i = argumentsCount; i > 0; --i) {
				const Script::Reference value = Script::Stack< Script::Reference >::Get(L, -i);

				const std::string result = tostring(value).Get< std::string >();

				printResult.emplace_back(result);
			}
			lua_pop(L, argumentsCount);
			return 0;
		},
	};
	script.SetGlobal("print", print);

	ASSERT_TRUE(script.ExecuteRaw("print('foo', 'bar', 'baz');"));

	EXPECT_THAT(printResult, ElementsAre("foo", "bar", "baz"));
}

class UnitScript_Metatable : public UnitScript
{
protected:
	class BaseClass
	{
	public:
		BaseClass(std::string param)
			: mParam(std::move(param))
		{
			CreatedObjects++;
		}
		virtual ~BaseClass()
		{
			CreatedObjects--;
		}

		[[nodiscard]] static auto StaticFunction() -> std::string { return "FooBar_Static"; }
		[[nodiscard]] auto GetFunction() const -> std::string { return mParam; }
		void SetFunction(const std::string& param) { mParam = param; }

		[[nodiscard]] auto AddFunction(const BaseClass* rhs) const -> std::string
		{
			return GetFunction() + "+" + rhs->GetFunction();
		}

	protected:
		std::string mParam = {};
	};

	class DerivedClass : public BaseClass
	{
	public:
		DerivedClass(std::string param)
			: BaseClass(std::move(param)) { }

		void SetFunction(const std::string& param) { mParam = param + "_Derived"; }
	};
};

class UnitScript_Class : public UnitScript_Metatable
{
protected:
	void SetUp() override
	{
		UnitScript_Metatable::SetUp();

		const std::string BaseMetatable = Script::Utils::DemangleClassName< BaseClass >();

		script.GetMetatable(BaseMetatable)
			->SetField("Create", std::function{ [](const std::string& param) {
				return new BaseClass{ param };
			} })
			->SetField("Destroy", std::function{ [](BaseClass* object) {
				delete object;
			} })
			->SetField("StaticFunction", &BaseClass::StaticFunction)
			->SetField("GetFunction", &BaseClass::GetFunction)
			->SetField("SetFunction", &BaseClass::SetFunction)
			->SetField("__add", &BaseClass::AddFunction);

		const std::string DerivedMetatable = Script::Utils::DemangleClassName< DerivedClass >();

		script.GetMetatable(DerivedMetatable, BaseMetatable)
			->SetField("Create", std::function{ [](const std::string& param) {
				return new DerivedClass{ param };
			} })
			->SetField("Destroy", std::function{ [](DerivedClass* object) {
				delete object;
			} })
			->SetField("SetFunction", &DerivedClass::SetFunction);

		ASSERT_TRUE(script.ExecuteRaw("BaseClass = " + BaseMetatable));
		ASSERT_TRUE(script.ExecuteRaw("DerivedClass = " + DerivedMetatable));
	}
};

TEST_F(UnitScript_Class, ShouldCallStaticFunction)
{
	EXPECT_EQ(script.Execute(R"(return BaseClass.StaticFunction())").Get< std::string >(), "FooBar_Static");
	EXPECT_EQ(script.Execute(R"(return DerivedClass.StaticFunction())").Get< std::string >(), "FooBar_Static");
}

TEST_F(UnitScript_Class, ShouldCreateBaseClassAndCallObjectFunctions)
{
	ASSERT_TRUE(script.ExecuteRaw(R"(Variable = BaseClass.Create("FooBar"))"));
	ASSERT_EQ(script[ "Variable" ].GetType(), Script::VariableType::LightUserData);

	EXPECT_EQ(script.Execute(R"(return Variable:GetFunction())").Get< std::string >(), "FooBar");
	EXPECT_TRUE(script.ExecuteRaw(R"(Variable:SetFunction("FooBar_Set"))"));
	EXPECT_EQ(script.Execute(R"(return Variable:GetFunction())").Get< std::string >(), "FooBar_Set");
	EXPECT_TRUE(script.ExecuteRaw(R"(Variable:Destroy())"));
}

TEST_F(UnitScript_Class, ShouldCreateDerivedClassAndCallObjectFunctions)
{
	ASSERT_TRUE(script.ExecuteRaw(R"(Variable = BaseClass.Create("FooBar"))"));
	ASSERT_EQ(script[ "Variable" ].GetType(), Script::VariableType::LightUserData);

	EXPECT_EQ(script.Execute(R"(return Variable:GetFunction())").Get< std::string >(), "FooBar");
	EXPECT_TRUE(script.ExecuteRaw(R"(Variable:SetFunction("FooBar_Set"))"));
	EXPECT_EQ(script.Execute(R"(return Variable:GetFunction())").Get< std::string >(), "FooBar_Set");
	EXPECT_TRUE(script.ExecuteRaw(R"(Variable:Destroy())"));
}

TEST_F(UnitScript_Class, ShouldCreateAndCallAddFunction)
{
	ASSERT_TRUE(script.ExecuteRaw(R"(Variable = DerivedClass.Create("FooBar"))"));

	EXPECT_EQ(script.Execute(R"(return Variable + Variable)").Get< std::string >(), "FooBar+FooBar");

	EXPECT_TRUE(script.ExecuteRaw(R"(Variable:Destroy())"));
}

class UnitScript_SharedClass : public UnitScript_Metatable
{
protected:
	using BaseClassPtr = std::shared_ptr< BaseClass >;
	using DerivedClassPtr = std::shared_ptr< DerivedClass >;

	void SetUp() override
	{
		UnitScript_Metatable::SetUp();

		const std::string BaseMetatable = Script::Utils::DemangleClassName< BaseClass >();

		script.GetMetatable(BaseMetatable)
			->RegisterReferenceDestructor(&script)
			->SetField("Create", std::function{ [](const std::string& param) {
				return BaseClassPtr{ new BaseClass{ param } };
			} })
			->SetField("StaticFunction", &BaseClass::StaticFunction)
			->SetField("GetFunction", &BaseClass::GetFunction)
			->SetField("SetFunction", &BaseClass::SetFunction);

		const std::string DerivedMetatable = Script::Utils::DemangleClassName< DerivedClass >();

		script.GetMetatable(DerivedMetatable, BaseMetatable)
			->RegisterReferenceDestructor(&script)
			->SetField("Create", std::function{ [](const std::string& param) {
				return DerivedClassPtr{ new DerivedClass{ param } };
			} })
			->SetField("SetFunction", &DerivedClass::SetFunction);

		ASSERT_TRUE(script.ExecuteRaw("BaseClass = " + BaseMetatable));
		ASSERT_TRUE(script.ExecuteRaw("DerivedClass = " + DerivedMetatable));
	}
};

TEST_F(UnitScript_SharedClass, ShouldCallStaticFunction)
{
	EXPECT_EQ(script.Execute(R"(return BaseClass.StaticFunction())").Get< std::string >(), "FooBar_Static");
	EXPECT_EQ(script.Execute(R"(return DerivedClass.StaticFunction())").Get< std::string >(), "FooBar_Static");
}

TEST_F(UnitScript_SharedClass, ShouldCreateBaseClassAndCallObjectFunctions)
{
	ASSERT_TRUE(script.ExecuteRaw(R"(Variable = BaseClass.Create("FooBar"))"));
	ASSERT_EQ(script[ "Variable" ].GetType(), Script::VariableType::UserData);

	EXPECT_EQ(script.Execute(R"(return Variable:GetFunction())").Get< std::string >(), "FooBar");
	EXPECT_TRUE(script.ExecuteRaw(R"(Variable:SetFunction("FooBar_Set"))"));
	EXPECT_EQ(script.Execute(R"(return Variable:GetFunction())").Get< std::string >(), "FooBar_Set");
	EXPECT_TRUE(script.ExecuteRaw(R"(Variable = nil)"));
}

TEST_F(UnitScript_SharedClass, ShouldCreateDerivedClassAndCallObjectFunctions)
{
	ASSERT_TRUE(script.ExecuteRaw(R"(Variable = DerivedClass.Create("FooBar"))"));
	ASSERT_EQ(script[ "Variable" ].GetType(), Script::VariableType::UserData);

	EXPECT_EQ(script.Execute(R"(return Variable:GetFunction())").Get< std::string >(), "FooBar");
	EXPECT_TRUE(script.ExecuteRaw(R"(Variable:SetFunction("FooBar_Set"))"));
	EXPECT_EQ(script.Execute(R"(return Variable:GetFunction())").Get< std::string >(), "FooBar_Set_Derived");
	EXPECT_TRUE(script.ExecuteRaw(R"(Variable = nil)"));
}

class UnitScript_ObjectClass : public UnitScript
{
protected:
	class BaseClass
		: virtual public Script::Object
	{
	public:
		BaseClass(std::string param)
			: mParam(std::move(param))
		{
			CreatedObjects++;
		}
		virtual ~BaseClass() override
		{
			CreatedObjects--;
		}

		[[nodiscard]] static auto Metatable() -> std::string_view { return "BaseClass"; }
		[[nodiscard]] virtual auto GetMetatable() const -> std::string_view override { return Metatable(); }

		[[nodiscard]] static auto StaticFunction() -> std::string { return "FooBar_Static"; }
		[[nodiscard]] auto GetFunction() const -> std::string { return mParam; }
		void SetFunction(const std::string& param) { mParam = param; }

	protected:
		std::string mParam = {};
	};

	class DerivedClass : public BaseClass
		, virtual public Script::Object
	{
	public:
		DerivedClass(std::string param)
			: BaseClass(std::move(param)) { }

		[[nodiscard]] static auto Metatable() -> std::string_view { return "DerivedClass"; }
		[[nodiscard]] virtual auto GetMetatable() const -> std::string_view override { return Metatable(); }

		void SetFunction(const std::string& param) { mParam = param + "_Derived"; }
	};

	using BaseClassPtr = std::shared_ptr< BaseClass >;
	using DerivedClassPtr = std::shared_ptr< DerivedClass >;

	void SetUp() override
	{
		script.GetMetatable(BaseClass::Metatable())
			->RegisterReferenceDestructor(&script)
			->SetField("new", std::function{ [](const std::string& param) {
				return BaseClassPtr{ new BaseClass{ param } };
			} })
			->SetField("GetMetatable", &BaseClass::GetMetatable)
			->SetField("StaticFunction", &BaseClass::StaticFunction)
			->SetField("GetFunction", &BaseClass::GetFunction)
			->SetField("SetFunction", &BaseClass::SetFunction);

		script.GetMetatable(DerivedClass::Metatable(), BaseClass::Metatable())
			->RegisterReferenceDestructor(&script)
			->SetField("Create", std::function{ [](const std::string& param) {
				return DerivedClassPtr{ new DerivedClass{ param } };
			} })
			->SetField("SetFunction", &DerivedClass::SetFunction);
	}
};

TEST_F(UnitScript_ObjectClass, ShouldCallStaticFunction)
{
	EXPECT_EQ(script.Execute(R"(return DerivedClass.StaticFunction())").Get< std::string >(), "FooBar_Static");
}

TEST_F(UnitScript_ObjectClass, ShouldCreateAndCallObjectFunctions)
{
	ASSERT_TRUE(script.ExecuteRaw(R"(Variable = DerivedClass.Create("FooBar"))"));
	ASSERT_EQ(script[ "Variable" ].GetType(), Script::VariableType::UserData);

	EXPECT_EQ(script.Execute(R"(return Variable:GetFunction())").Get< std::string >(), "FooBar");
	EXPECT_TRUE(script.ExecuteRaw(R"(Variable:SetFunction("FooBar_Set"))"));
	EXPECT_EQ(script.Execute(R"(return Variable:GetFunction())").Get< std::string >(), "FooBar_Set_Derived");
	EXPECT_TRUE(script.ExecuteRaw(R"(Variable = nil)"));
}

class UnitScript_Sandbox : public UnitScript
{
};

TEST_F(UnitScript_Sandbox, ShouldCreateAndAssignVariable)
{
	ASSERT_TRUE(
		script.GetSandbox("SandboxFirst")
			->SetField("Variable", 123)
			->Execute(R"(Variable = Variable + 1)"));

	ASSERT_TRUE(
		script.GetSandbox("SandboxSecond")
			->Execute(R"(Variable = SandboxFirst.Variable + 100)"));

	EXPECT_EQ(script[ "SandboxFirst" ][ "Variable" ].Get< int32_t >(), 124);
	EXPECT_EQ(script[ "SandboxSecond" ][ "Variable" ].Get< int32_t >(), 224);
}
