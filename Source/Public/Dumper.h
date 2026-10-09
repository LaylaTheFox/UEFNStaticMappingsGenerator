#pragma once
#include <unordered_map>
#include "PE.h"

class Dumper
{
public:
	static bool SetupHooks(PE Module);
	static bool DumpModule(PE Module);
	static void PrintInfo();

private:
	static void HandleMissingStructTypes();

public:
	class FPropertyParamsBase;
	struct FEnumeratorParam;
	struct CompactPropertyData;

	enum class ECppForm : uint8;

private:
	class UClass {};
	class UScriptStruct {};
	class UFunction {};
	class UEnum {};
	class FFieldClass {};
	class FCppClassTypeInfoStatic {};
	class FClassFunctionLinkInfo {};
	class FText {};
	class UObject {};
	using FTypeConstructFunc = UObject*(ETypeConstructPhase);

	class FImplementedInterfaceParams {};
	class FClassRegistrationInfo {};


	struct FClassReloadVersionInfo
	{
		SIZE_T Size = 0;
		uint32 Hash = 0;
	};

	struct FClassRegisterCompiledInInfo
	{
	public:
		class UClass* (*Register)(int);
		const TCHAR* Name;
		FClassRegistrationInfo* Info;
		FClassRegistrationInfo* Info2;
	};

	struct FStructRegisterCompiledInInfo
	{
	public:
		class UScriptStruct* (*OuterRegister)(int);
		void* (*CreateCppStructOps)();
		const TCHAR* Name;
		FClassRegistrationInfo* Info;
		FClassRegistrationInfo* Info2;
	};

	struct FEnumRegisterCompiledInInfo
	{
	public:
		class UEnum* (*OuterRegister)(int);
		const TCHAR* Name;
		FClassRegistrationInfo* Info;
		FClassRegistrationInfo* Info2;
	};

	struct FPartialRegisterCompiledInInfo {};

	struct FClassParams
	{
	public:
		UClass* (*ClassNoRegisterFunc)(int);
		const char* ClassConfigNameUTF8;
		const FCppClassTypeInfoStatic* CppClassInfo;
		FTypeConstructFunc** DependencySingletonFuncArray;
		const FClassFunctionLinkInfo* FunctionLinkArray;
		const FPropertyParamsBase* const* PropertyArray;
		const FImplementedInterfaceParams* ImplementedInterfaceArray;
		uint32                                      NumDependencySingletons : 4;
		uint32                                      NumFunctions : 11;
		uint32                                      NumProperties : 11;
		uint32                                      NumImplementedInterfaces : 6;
		uint32                                      ClassFlags; // EClassFlags
#if WITH_METADATA
		uint16                                      NumMetaData;
		const FMetaDataPairParam* MetaDataArray;
#endif
	};

	struct FStructParams
	{
	public:
		void* OuterFunc;
		UScriptStruct* (*SuperFunc)(int);
		void* (*StructOpsFunc)(); // really returns UScriptStruct::ICppStructOps*
		const char* NameUTF8;
		const FPropertyParamsBase* const* PropertyArray;
		uint16                            NumProperties;
		uint16                            SizeOf;
		uint8                             AlignOf;
		EObjectFlags                      ObjectFlags;
		uint32                            StructFlags; // EStructFlags
#if WITH_METADATA
		uint16                              NumMetaData;
		const FMetaDataPairParam* MetaDataArray;
#endif
	};

	struct FEnumParams
	{
		void* OuterFunc;
		FText(*DisplayNameFunc)(int32);
		const char* NameUTF8;
		const char* CppTypeUTF8;
		const FEnumeratorParam* EnumeratorParams;
		EObjectFlags                ObjectFlags;
		int16                       NumEnumerators;
		EEnumFlags                  EnumFlags;
		ECppForm                    CppForm; // this is of type UEnum::ECppForm
		uint8                       UnderlyingType; // this is of type UEnum::EUnderlyingType
#if WITH_METADATA
		uint16                      NumMetaData;
		const FMetaDataPairParam* MetaDataArray;
#endif
	};

	static void RegisterCompiledInInfo(const TCHAR* PackageName, const FClassRegisterCompiledInInfo* ClassInfo, size_t NumClassInfo, const FStructRegisterCompiledInInfo* StructInfo, size_t NumStructInfo, const FEnumRegisterCompiledInInfo* EnumInfo, size_t NumEnumInfo, const FPartialRegisterCompiledInInfo* PartialsInfo, size_t NumPartialsInfo);
	static void ConstructUClass(const char** OutClass, const FClassParams* Params);
	static void ConstructUScriptStruct(const char** OutStruct, const FStructParams* Params);
	static void ConstructUEnum(const char** OutEnum, const FEnumParams* Params);
	
	static void SetSparseClassDataStruct();

	static std::vector<CompactPropertyData> ParseProperties(const FPropertyParamsBase* const* PropertyParams, int32 NumProperties);
	static void ParseProperty(CompactPropertyData& PropertyData, const FPropertyParamsBase* const* PropertyParams, int32& NumProperties);

private:
	static inline const char* RegisterCompiledInInfoName = "?RegisterCompiledInInfo@@YAXPEB_WPEBUFClassRegisterCompiledInInfo@@_KPEBUFStructRegisterCompiledInInfo@@2PEBUFEnumRegisterCompiledInInfo@@2PEBUFPartialRegisterCompiledInInfo@@2@Z";
	static inline const char* ConstructUClassName = "?ConstructUClass@UECodeGen_Private@@YAXAEAPEAVUClass@@AEBUFClassParams@1@@Z";
	static inline const char* ConstructUScriptStructName = "?ConstructUScriptStruct@UECodeGen_Private@@YAXAEAPEAVUScriptStruct@@AEBUFStructParams@1@@Z";
	static inline const char* ConstructUEnumName = "?ConstructUEnum@UECodeGen_Private@@YAXAEAPEAVUEnum@@AEBUFEnumParams@1@@Z";

	static inline const char* SetSparseClassDataStructName = "?SetSparseClassDataStruct@UClass@@QEAAXPEAVUScriptStruct@@@Z";

	
public:
	enum class EPropertyGenFlags : uint8
	{
		None = 0x00,

		// First 6 bits are the property type
		Byte = 0x00,
		Int8 = 0x01,
		Int16 = 0x02,
		Int = 0x03,
		Int64 = 0x04,
		UInt16 = 0x05,
		UInt32 = 0x06,
		UInt64 = 0x07,
		//                = 0x08,
		//                = 0x09,
		Float = 0x0A,
		Double = 0x0B,
		Bool = 0x0C,
		SoftClass = 0x0D,
		WeakObject = 0x0E,
		LazyObject = 0x0F,
		SoftObject = 0x10,
		Class = 0x11,
		Object = 0x12,
		Interface = 0x13,
		Name = 0x14,
		Str = 0x15,
		Array = 0x16,
		Map = 0x17,
		Set = 0x18,
		Struct = 0x19,
		Delegate = 0x1A,
		InlineMulticastDelegate = 0x1B,
		SparseMulticastDelegate = 0x1C,
		Text = 0x1D,
		Enum = 0x1E,
		FieldPath = 0x1F,
		LargeWorldCoordinatesReal = 0x20,
		Optional = 0x21,
		VerseValue = 0x22,
		Utf8Str = 0x23,
		AnsiStr = 0x24,
		VerseString = 0x25,
		VerseCell = 0x26,
		VerseType = 0x27,
		VerseLegacyValue = 0x28, // FVRestValueProperty with a legacy property

		// Property-specific flags
		NativeBool = 0x40,
		ObjectPtr = 0x40,

	};

	enum EPropertyFlags : uint64
	{
		CPF_None = 0,

		CPF_Edit = 0x0000000000000001,	///< Property is user-settable in the editor.
		CPF_ConstParm = 0x0000000000000002,	///< This is a constant function parameter
		CPF_BlueprintVisible = 0x0000000000000004,	///< This property can be read by blueprint code
		CPF_ExportObject = 0x0000000000000008,	///< Object can be exported with actor.
		CPF_BlueprintReadOnly = 0x0000000000000010,	///< This property cannot be modified by blueprint code
		CPF_Net = 0x0000000000000020,	///< Property is relevant to network replication.
		CPF_EditFixedSize = 0x0000000000000040,	///< Indicates that elements of an array can be modified, but its size cannot be changed.
		CPF_Parm = 0x0000000000000080,	///< Function/When call parameter.
		CPF_OutParm = 0x0000000000000100,	///< Value is copied out after function call.
		CPF_ZeroConstructor = 0x0000000000000200,	///< memset is fine for construction
		CPF_ReturnParm = 0x0000000000000400,	///< Return value.
		CPF_DisableEditOnTemplate = 0x0000000000000800,	///< Disable editing of this property on an archetype/sub-blueprint
		CPF_NonNullable = 0x0000000000001000,	///< Object property can never be null
		CPF_Transient = 0x0000000000002000,	///< Property is transient: shouldn't be saved or loaded, except for Blueprint CDOs.
		CPF_Config = 0x0000000000004000,	///< Property should be loaded/saved as permanent profile.
		CPF_RequiredParm = 0x0000000000008000,	///< Parameter must be linked explicitly in blueprint. Leaving the parameter out results in a compile error. 
		CPF_DisableEditOnInstance = 0x0000000000010000,	///< Disable editing on an instance of this class
		CPF_EditConst = 0x0000000000020000,	///< Property is uneditable in the editor.
		CPF_GlobalConfig = 0x0000000000040000,	///< Load config from base class, not subclass.
		CPF_InstancedReference = 0x0000000000080000,	///< Property is a component references.
		CPF_ExperimentalExternalObjects = 0x0000000000100000,	///< Property saves objects in separate files, breaks hard links and reload them based on discovery.
		CPF_DuplicateTransient = 0x0000000000200000,	///< Property should always be reset to the default value during any type of duplication (copy/paste, binary duplication, etc.)
		//CPF_								= 0x0000000000400000,	///< 
		//CPF_    							= 0x0000000000800000,	///< 
		CPF_SaveGame = 0x0000000001000000,	///< Property should be serialized for save games, this is only checked for game-specific archives with ArIsSaveGame
		CPF_NoClear = 0x0000000002000000,	///< Hide clear button.
		CPF_Virtual = 0x0000000004000000,	///< Property is defined on an interface and does not include a useful Offset_Internal.
		CPF_ReferenceParm = 0x0000000008000000,	///< Value is passed by reference; CPF_OutParam and CPF_Param should also be set.
		CPF_BlueprintAssignable = 0x0000000010000000,	///< MC Delegates only.  Property should be exposed for assigning in blueprint code
		CPF_Deprecated = 0x0000000020000000,	///< Property is deprecated.  Read it from an archive, but don't save it.
		CPF_IsPlainOldData = 0x0000000040000000,	///< If this is set, then the property can be memcopied instead of CopyCompleteValue / CopySingleValue
		CPF_RepSkip = 0x0000000080000000,	///< Not replicated. For non replicated properties in replicated structs 
		CPF_RepNotify = 0x0000000100000000,	///< Notify actors when a property is replicated
		CPF_Interp = 0x0000000200000000,	///< interpolatable property for use with cinematics
		CPF_NonTransactional = 0x0000000400000000,	///< Property isn't transacted
		CPF_EditorOnly = 0x0000000800000000,	///< Property should only be loaded in the editor
		CPF_NoDestructor = 0x0000001000000000,	///< No destructor
		//CPF_								= 0x0000002000000000,	///<
		CPF_AutoWeak = 0x0000004000000000,	///< Only used for weak pointers, means the export type is autoweak
		CPF_ContainsInstancedReference = 0x0000008000000000,	///< Property contains component references.
		CPF_AssetRegistrySearchable = 0x0000010000000000,	///< asset instances will add properties with this flag to the asset registry automatically
		CPF_SimpleDisplay = 0x0000020000000000,	///< The property is visible by default in the editor details view
		CPF_AdvancedDisplay = 0x0000040000000000,	///< The property is advanced and not visible by default in the editor details view
		CPF_Protected = 0x0000080000000000,	///< property is protected from the perspective of script
		CPF_BlueprintCallable = 0x0000100000000000,	///< MC Delegates only.  Property should be exposed for calling in blueprint code
		CPF_BlueprintAuthorityOnly = 0x0000200000000000,	///< MC Delegates only.  This delegate accepts (only in blueprint) only events with BlueprintAuthorityOnly.
		CPF_TextExportTransient = 0x0000400000000000,	///< Property shouldn't be exported to text format (e.g. copy/paste)
		CPF_NonPIEDuplicateTransient = 0x0000800000000000,	///< Property should only be copied in PIE
		CPF_ExposeOnSpawn = 0x0001000000000000,	///< Property is exposed on spawn
		CPF_PersistentInstance = 0x0002000000000000,	///< A object referenced by the property is duplicated like a component. (Each actor should have an own instance.)
		CPF_UObjectWrapper = 0x0004000000000000,	///< Property was parsed as a wrapper class like TSubclassOf<T>, FScriptInterface etc., rather than a USomething*
		CPF_HasGetValueTypeHash = 0x0008000000000000,	///< This property can generate a meaningful hash value.
		CPF_NativeAccessSpecifierPublic = 0x0010000000000000,	///< Public native access specifier
		CPF_NativeAccessSpecifierProtected = 0x0020000000000000,	///< Protected native access specifier
		CPF_NativeAccessSpecifierPrivate = 0x0040000000000000,	///< Private native access specifier
		CPF_SkipSerialization = 0x0080000000000000,	///< Property shouldn't be serialized, can still be exported to text
		CPF_TObjectPtr = 0x0100000000000000,	///< Property is a TObjectPtr<T> instead of a USomething*. Need to differentiate between TObjectclassOf and TObjectPtr
		CPF_ExperimentalOverridableLogic = 0x0200000000000000,	///< ****Experimental*** Property will use different logic to serialize knowing what changes are done against its default use the overridable information provided by the overridable manager on the object
		CPF_ExperimentalAlwaysOverriden = 0x0400000000000000,	///< ****Experimental*** Property should never inherit from the parent when using overridable serialization
		CPF_ExperimentalNeverOverriden = 0x0800000000000000,	///< ****Experimental*** Property should never be overridden when using overridable serialization
		CPF_AllowSelfReference = 0x1000000000000000,	///< Enables the instancing graph self referencing logic, delegates and verse function are already using this
		CPF_ForcePostConstructLink = 0x2000000000000000,	///< Use to force native UCLASS properties to be included in the PostConstructLink chain in a derived Blueprint class, regardless of whether it differs from the native class constructor-initialized value

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// DO NOT ADD composite flags to EPropertyFlags. Debugger visualizations have trouble displaying composite flag values as text.
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	};

	enum class ECppForm : uint8
	{
		Regular,
		Namespaced,
		EnumClass
	};


	typedef void (*SetterFuncPtr)(void* InContainer, const void* InValue);
	typedef void (*GetterFuncPtr)(const void* InContainer, void* OutValue);

#define UE_PROPERTY_PARAMS_BASE_FIELDS \
	const char*					NameUTF8; \
	const char*					RepNotifyFuncUTF8; \
	EPropertyFlags				PropertyFlags; \
	EPropertyGenFlags			Flags; \
	SetterFuncPtr				SetterFunc; \
	GetterFuncPtr				GetterFunc; \
	uint16						ArrayDim;

#define UE_PROPERTY_PARAMS_BASE_WITHOUT_OFFSET_FIELDS \
	UE_PROPERTY_PARAMS_BASE_FIELDS

#define UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS \
	UE_PROPERTY_PARAMS_BASE_FIELDS \
	uint16						Offset;

	class FPropertyParamsBase
	{
	public:
		UE_PROPERTY_PARAMS_BASE_FIELDS
	};

	class FPropertyParamsBaseWithOffset
	{
	public:
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS
	};

	struct FGenericPropertyParams // : FPropertyParamsBaseWithOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
	};

	struct FBytePropertyParams // : FPropertyParamsBaseWithOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		UEnum* (*EnumFunc)(ETypeConstructPhase);
	};

	struct FBoolPropertyParams // : FPropertyParamsBaseWithoutOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITHOUT_OFFSET_FIELDS;
		uint16					ElementSize;
		uint16					SizeOfOuter;
		void					(*SetBitFunc)(void* Obj);
	};

	struct FObjectPropertyParams // : FPropertyParamsBaseWithOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		UClass* (*ClassFunc)(ETypeConstructPhase);
	};

	struct FClassPropertyParams // : FObjectPropertyParams
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		UClass* (*ClassFunc)(ETypeConstructPhase);
		UClass* (*MetaClassFunc)(ETypeConstructPhase);
	};

	struct FVerseClassPropertyParams // : FClassPropertyParams
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		UClass* (*ClassFunc)(ETypeConstructPhase);
		UClass* (*MetaClassFunc)(ETypeConstructPhase);
		bool					bRequiresConcrete;
		bool					bRequiresCastable;
	};

	struct FSoftClassPropertyParams // : FObjectPropertyParamsWithoutClass
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		UClass* (*MetaClassFunc)(ETypeConstructPhase);
	};

	struct FInterfacePropertyParams // : FPropertyParamsBaseWithOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		UClass* (*InterfaceClassFunc)(ETypeConstructPhase);
	};

	struct FStructPropertyParams // : FPropertyParamsBaseWithOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		UScriptStruct* (*ScriptStructFunc)(ETypeConstructPhase);
	};

	struct FDelegatePropertyParams // : FPropertyParamsBaseWithOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		UFunction* (*SignatureFunctionFunc)(ETypeConstructPhase);
	};

	struct FMulticastDelegatePropertyParams // : FPropertyParamsBaseWithOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		UFunction* (*SignatureFunctionFunc)(ETypeConstructPhase);
	};

	struct FEnumPropertyParams // : FPropertyParamsBaseWithOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		UEnum* (*EnumFunc)(ETypeConstructPhase);
	};

	struct FFieldPathPropertyParams // : FPropertyParamsBaseWithOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		FFieldClass* (*PropertyClassFunc)();
	};

	struct FArrayPropertyParams // : FPropertyParamsBaseWithOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		EArrayPropertyFlags		ArrayFlags;
	};

	struct FMapPropertyParams // : FPropertyParamsBaseWithOffset
	{
		UE_PROPERTY_PARAMS_BASE_WITH_OFFSET_FIELDS;
		EMapPropertyFlags		MapFlags;
	};

	struct FEnumeratorParam
	{
		const char* NameUTF8;
		int64                     Value;
	};

	struct CompactPropertyData
	{
	public:
		std::string PropertyName;
		EPropertyFlags PropertyFlags;
		EPropertyGenFlags Flags;
		uint16 ArrayDim;

		const FPropertyParamsBase* RawProperty;
		std::vector<CompactPropertyData> PropertyLinks;
	};

	struct CompactClassData 
	{
	public:
		std::string ClassName;
		std::vector<CompactPropertyData> Properties;
		FTypeConstructFunc* DependencySingletonFunction;
	};

	struct CompactStructData
	{
	public:
		std::string StructName;
		std::vector<CompactPropertyData> Properties;
		UScriptStruct* (*SuperFunc)(int);
	};

	class ClassStructWrapper {
	public:
		ClassStructWrapper() {
			IsValid = false;
		}

		ClassStructWrapper(CompactClassData ClassData) {
			IsValid = true;
			IsStruct = false;
			Name = ClassData.ClassName;
			LocalProperties = ClassData.Properties;
			DependencySingletonFunction = ClassData.DependencySingletonFunction;
		}

		ClassStructWrapper(CompactStructData StructData) {
			IsValid = true;
			IsStruct = true;
			Name = StructData.StructName;
			LocalProperties = StructData.Properties;
			SuperFunc = StructData.SuperFunc;
		}

		ClassStructWrapper GetSuper() {
			if (IsStruct && SuperFunc) 
			{
				char* StructSuperName = (char*)SuperFunc(0); //must be 0
				if (!StructSuperName)
				{
					std::cout << "Failed to find super for: " << Name << "\n";
					return ClassStructWrapper();
				}
				
				auto Result = StructDataMap.find(StructSuperName);
				if (Result == StructDataMap.end()) {
					std::cout << "Failed to find struct: " << StructSuperName << "\n";
					return ClassStructWrapper();
				}

				return ClassStructWrapper(Result->second);
			}
			else if(DependencySingletonFunction)
			{
				auto Result = ClassDataMap.find(DependencySingletonFunction);
				if (Result == ClassDataMap.end()) 
				{
					std::cout << "Failed to find class super for: " << Name << "\n";
					return ClassStructWrapper();
				}

				return ClassStructWrapper(Result->second);
			}

			return ClassStructWrapper();
		}

		std::vector<CompactPropertyData> GetAllProperties() {
			std::vector<CompactPropertyData> Properties;

			auto Super = GetSuper();
			if (Super.IsValid) {
				Properties = Super.GetAllProperties();
			}

			for (CompactPropertyData Property : LocalProperties) {
				Properties.push_back(Property);
			}

			return Properties;
		}
		std::vector<CompactPropertyData> GetLocalProperties(bool FilterEditorOnly = true) 
		{	
			std::vector<CompactPropertyData> Properties;

			for (CompactPropertyData Property : LocalProperties) {
				if (FilterEditorOnly && Property.PropertyFlags & EPropertyFlags::CPF_EditorOnly)
					continue;

				Properties.push_back(Property);
			}

			return Properties;
		}

		std::string GetStrippedName() { return IsStruct ? Name : Name.substr(1); }
		std::string GetRawName() { return Name; }
		bool GetIsValid() { return IsValid; }

	private:
		bool IsValid = false;
		bool IsStruct = false;

		std::string Name = "";
		std::vector<CompactPropertyData> LocalProperties = {};

		FTypeConstructFunc* DependencySingletonFunction = nullptr;
		UScriptStruct* (*SuperFunc)(int) = nullptr;
	};

	struct CompactEnumData
	{
	public:
		ECppForm CppForm;
		uint16 NumProperties;
		const FEnumeratorParam* PropertyArray;
	};

	static inline std::vector<const FPropertyParamsBase*> MissingStructTypes;

	static inline std::unordered_map<void*, CompactClassData> ClassDataMap;
	static inline std::unordered_map<std::string, CompactStructData> StructDataMap;
	static inline std::unordered_map<std::string, CompactEnumData> EnumDataMap;
};

