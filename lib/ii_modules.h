#pragma once

// TODO this string is auto-generated from the i2c devices library
// this is the response to 'II.listmodules'
const char* ii_module_list = "--- II: supported modules\n\r"
    "jf\t--just friends\n\r"
    "wslash\t--w/\n\r"
    "ansible\t--ansible\n\r"
    "tt\t--teletype\n\r"
// ADD YOUR NEWLY SUPPORTED MODULE ABOVE THIS LINE
    "\n\r"
    "--- See available commands with 'II.<module>.listcommands()', eg:\n\r"
    "II.jf.listmodules()\n\r"
    ;

// full name of module  just friends
// manufacturer(?)      mannequins
// lua name for module  jf
// i2c address          0xA0 (?)
// list of functions    <raw text. 1fn per line. copyable to lua script>

typedef enum{ II_void
            , II_u8
            , II_s8
            , II_u16
            , II_s16
            , II_float   // 32bit (for crow to crow comm'n)
} II_arg_type;

// TODO how to handle getters v setters?
typedef struct{
    //const char* "transpose( semitones )" // lua prototype
    //jf_transpose             // c function TODO how to structure the Lua->C call?
    const char*       prototype;
    const uint8_t     arg_count;
    const II_arg_type args[];
    const II_arg_type return_type;
} II_fn_t;

typedef struct{
    const char*   fullname; // the name by which the module is known
    const char*   mfg;      // manufacturer who builds the module
    const char*   luaname;  // how the user will refer to the module in a lua script
    const uint8_t address;  // i2c address by which module is addressable
    const II_fn_t fn_list[];  // list of function prototypes
} II_Module_t;

// TODO below should be declared in a lua script, which generates the below
II_Module_t jf = { .fullname = "just friends"
                 , .mfg      = "mannequins"
                 , .luaname  = "jf"
                 , .address  = 0xA0
                 , .fn_list  =
    { { "transpose( semitones )"
        , 1, {II_s16}
        , II_void }
    , { "note( pitch, level )"
        , 2, {II_s16, II_s16}
        , II_void }
    }
};

