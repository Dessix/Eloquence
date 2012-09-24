const char* JSONLIBLUA = "_JSON = (function() "
	"local VERSION = 20111207.5\n"
	"local OBJDEF = { VERSION = VERSION }\n"
	"local author = \"-[ JSON.lua package by Jeffrey Friedl (http://regex.info/blog/lua/json), version \" .. tostring(VERSION) .. \" ]-\"\n"
	"local isArray  = { __tostring = function() return \"JSON array\"  end }    isArray.__index  = isArray\n"
	"local isObject = { __tostring = function() return \"JSON object\" end }    isObject.__index = isObject\n"
	"function OBJDEF:newArray(tbl)\n"
	"return setmetatable(tbl or {}, isArray)\n"
	"end\n"
	"function OBJDEF:newObject(tbl)\n"
	"return setmetatable(tbl or {}, isObject)\n"
	"end\n"
	"local function unicode_codepoint_as_utf8(codepoint)\n"
	"if codepoint <= 127 then\n"
	"return string.char(codepoint)\n"
	"elseif codepoint <= 2047 then\n"
	"local highpart = math.floor(codepoint / 0x40)\n"
	"local lowpart  = codepoint - (0x40 * highpart)\n"
	"return string.char(0xC0 + highpart,\n"
	"0x80 + lowpart)\n"
	"elseif codepoint <= 65535 then\n"
	"local highpart  = math.floor(codepoint / 0x1000)\n"
	"local remainder = codepoint - 0x1000 * highpart\n"
	"local midpart   = math.floor(remainder / 0x40)\n"
	"local lowpart   = remainder - 0x40 * midpart\n"
	"highpart = 0xE0 + highpart\n"
	"midpart  = 0x80 + midpart\n"
	"lowpart  = 0x80 + lowpart\n"
	"if ( highpart == 0xE0 and midpart < 0xA0 ) or\n"
	"( highpart == 0xED and midpart > 0x9F ) or\n"
	"( highpart == 0xF0 and midpart < 0x90 ) or\n"
	"( highpart == 0xF4 and midpart > 0x8F )\n"
	"then\n"
	"return \"?\"\n"
	"else\n"
	"return string.char(highpart,\n"
	"midpart,\n"
	"lowpart)\n"
	"end\n"
	"else\n"
	"local highpart  = math.floor(codepoint / 0x40000)\n"
	"local remainder = codepoint - 0x40000 * highpart\n"
	"local midA      = math.floor(remainder / 0x1000)\n"
	"remainder       = remainder - 0x1000 * midA\n"
	"local midB      = math.floor(remainder / 0x40)\n"
	"local lowpart   = remainder - 0x40 * midB\n"
	"return string.char(0xF0 + highpart,\n"
	"0x80 + midA,\n"
	"0x80 + midB,\n"
	"0x80 + lowpart)\n"
	"end\n"
	"end\n"
	"function OBJDEF:onDecodeError(message, text, location, etc)\n"
	"if text then\n"
	"if location then\n"
	"message = string.format(\"%s at char %d of: %s\", message, location, text)\n"
	"else\n"
	"message = string.format(\"%s: %s\", message, text)\n"
	"end\n"
	"end\n"
	"if etc ~= nil then\n"
	"message = message .. \" (\" .. OBJDEF:encode(etc) .. \")\"\n"
	"end\n"
	"if self.assert then\n"
	"self.assert(false, message)\n"
	"else\n"
	"assert(false, message)\n"
	"end\n"
	"end\n"
	"OBJDEF.onDecodeOfNilError  = OBJDEF.onDecodeError\n"
	"OBJDEF.onDecodeOfHTMLError = OBJDEF.onDecodeError\n"
	"function OBJDEF:onEncodeError(message, etc)\n"
	"if etc ~= nil then\n"
	"message = message .. \" (\" .. OBJDEF:encode(etc) .. \")\"\n"
	"end\n"
	"if self.assert then\n"
	"self.assert(false, message)\n"
	"else\n"
	"assert(false, message)\n"
	"end\n"
	"end\n"
	"local function grok_number(self, text, start, etc)\n"
	"local integer_part = text:match(\'^-?[1-9]%d*\', start)\n"
	"or text:match(\"^-?0\",        start)\n"
	"if not integer_part then\n"
	"self:onDecodeError(\"expected number\", text, start, etc)\n"
	"end\n"
	"local i = start + integer_part:len()\n"
	"local decimal_part = text:match(\'^%.%d+\', i) or \"\"\n"
	"i = i + decimal_part:len()\n"
	"local exponent_part = text:match(\'^[eE][-+]?%d+\', i) or \"\"\n"
	"i = i + exponent_part:len()\n"
	"local full_number_text = integer_part .. decimal_part .. exponent_part\n"
	"local as_number = tonumber(full_number_text)\n"
	"if not as_number then\n"
	"self:onDecodeError(\"bad number\", text, start, etc)\n"
	"end\n"
	"return as_number, i\n"
	"end\n"
	"local function grok_string(self, text, start, etc)\n"
	"if text:sub(start,start) ~= \'\"\' then\n"
	"self:onDecodeError(\"expected string\'s opening quote\", text, start, etc)\n"
	"end\n"
	"local i = start + 1\n"
	"local text_len = text:len()\n"
	"local VALUE = \"\"\n"
	"while i <= text_len do\n"
	"local c = text:sub(i,i)\n"
	"if c == \'\"\' then\n"
	"return VALUE, i + 1\n"
	"end\n"
	"if c ~= \'\\\\\' then\n"
	"VALUE = VALUE .. c\n"
	"i = i + 1\n"
	"elseif text:match(\'^\\\\b\', i) then\n"
	"VALUE = VALUE .. \"\\b\"\n"
	"i = i + 2\n"
	"elseif text:match(\'^\\\\f\', i) then\n"
	"VALUE = VALUE .. \"\\f\"\n"
	"i = i + 2\n"
	"elseif text:match(\'^\\\\n\', i) then\n"
	"VALUE = VALUE .. \"\\n\"\n"
	"i = i + 2\n"
	"elseif text:match(\'^\\\\r\', i) then\n"
	"VALUE = VALUE .. \"\\r\"\n"
	"i = i + 2\n"
	"elseif text:match(\'^\\\\t\', i) then\n"
	"VALUE = VALUE .. \"\\t\"\n"
	"i = i + 2\n"
	"else\n"
	"local hex = text:match(\'^\\\\u([0123456789aAbBcCdDeEfF][0123456789aAbBcCdDeEfF][0123456789aAbBcCdDeEfF][0123456789aAbBcCdDeEfF])\', i)\n"
	"if hex then\n"
	"i = i + 6\n"
	"local codepoint = tonumber(hex, 16)\n"
	"if codepoint >= 0xD800 and codepoint <= 0xDBFF then\n"
	"local lo_surrogate = text:match(\'^\\\\u([dD][cdefCDEF][0123456789aAbBcCdDeEfF][0123456789aAbBcCdDeEfF])\', i)\n"
	"if lo_surrogate then\n"
	"i = i + 6\n"
	"codepoint = 0x2400 + (codepoint - 0xD800) * 0x400 + tonumber(lo_surrogate, 16)\n"
	"else\n"
	"end\n"
	"end\n"
	"VALUE = VALUE .. unicode_codepoint_as_utf8(codepoint)\n"
	"else\n"
	"VALUE = VALUE .. text:match(\'^\\\\(.)\', i)\n"
	"i = i + 2\n"
	"end\n"
	"end\n"
	"end\n"
	"self:onDecodeError(\"unclosed string\", text, start, etc)\n"
	"end\n"
	"local function skip_whitespace(text, start)\n"
	"local match_start, match_end = text:find(\"^[ \\n\\r\\t]+\", start)\n"
	"if match_end then\n"
	"return match_end + 1\n"
	"else\n"
	"return start\n"
	"end\n"
	"end\n"
	"local grok_one\n"
	"local function grok_object(self, text, start, etc)\n"
	"if not text:sub(start,start) == \'{\' then\n"
	"self:onDecodeError(\"expected \'{\'\", text, start, etc)\n"
	"end\n"
	"local i = skip_whitespace(text, start + 1)\n"
	"local VALUE = self.strictTypes and self:newObject { } or { }\n"
	"if text:sub(i,i) == \'}\' then\n"
	"return VALUE, i + 1\n"
	"end\n"
	"local text_len = text:len()\n"
	"while i <= text_len do\n"
	"local key, new_i = grok_string(self, text, i, etc)\n"
	"i = skip_whitespace(text, new_i)\n"
	"if text:sub(i, i) ~= \':\' then\n"
	"self:onDecodeError(\"expected colon\", text, i, etc)\n"
	"end\n"
	"i = skip_whitespace(text, i + 1)\n"
	"local val, new_i = grok_one(self, text, i)\n"
	"VALUE[key] = val\n"
	"i = skip_whitespace(text, new_i)\n"
	"local c = text:sub(i,i)\n"
	"if c == \'}\' then\n"
	"return VALUE, i + 1\n"
	"end\n"
	"if text:sub(i, i) ~= \',\' then\n"
	"self:onDecodeError(\"expected comma or \'}\'\", text, i, etc)\n"
	"end\n"
	"i = skip_whitespace(text, i + 1)\n"
	"end\n"
	"self:onDecodeError(\"unclosed \'{\'\", text, start, etc)\n"
	"end\n"
	"local function grok_array(self, text, start, etc)\n"
	"if not text:sub(start,start) == \'[\' then\n"
	"self:onDecodeError(\"expected \'[\'\", text, start, etc)\n"
	"end\n"
	"local i = skip_whitespace(text, start + 1)\n"
	"local VALUE = self.strictTypes and self:newArray { } or { }\n"
	"if text:sub(i,i) == \']\' then\n"
	"return VALUE, i + 1\n"
	"end\n"
	"local text_len = text:len()\n"
	"while i <= text_len do\n"
	"local val, new_i = grok_one(self, text, i)\n"
	"table.insert(VALUE, val)\n"
	"i = skip_whitespace(text, new_i)\n"
	"local c = text:sub(i,i)\n"
	"if c == \']\' then\n"
	"return VALUE, i + 1\n"
	"end\n"
	"if text:sub(i, i) ~= \',\' then\n"
	"self:onDecodeError(\"expected comma or \'[\'\", text, i, etc)\n"
	"end\n"
	"i = skip_whitespace(text, i + 1)\n"
	"end\n"
	"self:onDecodeError(\"unclosed \'[\'\", text, start, etc)\n"
	"end\n"
	"grok_one = function(self, text, start, etc)\n"
	"start = skip_whitespace(text, start)\n"
	"if start > text:len() then\n"
	"self:onDecodeError(\"unexpected end of string\", text, nil, etc)\n"
	"end\n"
	"if text:find(\'^\"\', start) then\n"
	"return grok_string(self, text, start, etc)\n"
	"elseif text:find(\'^[-0123456789 ]\', start) then\n"
	"return grok_number(self, text, start, etc)\n"
	"elseif text:find(\'^%{\', start) then\n"
	"return grok_object(self, text, start, etc)\n"
	"elseif text:find(\'^%[\', start) then\n"
	"return grok_array(self, text, start, etc)\n"
	"elseif text:find(\'^true\', start) then\n"
	"return true, start + 4\n"
	"elseif text:find(\'^false\', start) then\n"
	"return false, start + 5\n"
	"elseif text:find(\'^null\', start) then\n"
	"return nil, start + 4\n"
	"else\n"
	"self:onDecodeError(\"can\'t parse JSON\", text, start, etc)\n"
	"end\n"
	"end\n"
	"function OBJDEF:decode(text, etc)\n"
	"if type(self) ~= \'table\' or self.__index ~= OBJDEF then\n"
	"OBJDEF:onDecodeError(\"JSON:decode must be called in method format\", nil, nil, etc)\n"
	"end\n"
	"if text == nil then\n"
	"self:onDecodeOfNilError(string.format(\"nil passed to JSON:decode()\"), nil, nil, etc)\n"
	"elseif type(text) ~= \'string\' then\n"
	"self:onDecodeError(string.format(\"expected string argument to JSON:decode(), got %s\", type(text)), nil, nil, etc)\n"
	"end\n"
	"if text:match(\'^%s*$\') then\n"
	"return nil\n"
	"end\n"
	"if text:match(\'^%s*<\') then\n"
	"self:onDecodeOfHTMLError(string.format(\"html passed to JSON:decode()\"), text, nil, etc)\n"
	"end\n"
	"if text:sub(1,1):byte() == 0 or (text:len() >= 2 and text:sub(2,2):byte() == 0) then\n"
	"self:onDecodeError(\"JSON package groks only UTF-8, sorry\", text, nil, etc)\n"
	"end\n"
	"local success, value = pcall(grok_one, self, text, 1, etc)\n"
	"if success then\n"
	"return value\n"
	"else\n"
	"assert(false, value)\n"
	"return nil\n"
	"end\n"
	"end\n"
	"local function backslash_replacement_function(c)\n"
	"if c == \"\\n\" then\n"
	"return \"\\\\n\"\n"
	"elseif c == \"\\r\" then\n"
	"return \"\\\\r\"\n"
	"elseif c == \"\\t\" then\n"
	"return \"\\\\t\"\n"
	"elseif c == \"\\b\" then\n"
	"return \"\\\\b\"\n"
	"elseif c == \"\\f\" then\n"
	"return \"\\\\f\"\n"
	"elseif c == \'\"\' then\n"
	"return \'\\\\\"\'\n"
	"elseif c == \'\\\\\' then\n"
	"return \'\\\\\\\\\'\n"
	"else\n"
	"return string.format(\"\\\\u%04x\", c:byte())\n"
	"end\n"
	"end\n"
	"local chars_to_be_escaped_in_JSON_string\n"
	"= \'[\'\n"
	"..    \'\"\'\n"
	"..    \'%\\\\\'\n"
	"..    \'%z\'\n"
	"..    \'\\001\' .. \'-\' .. \'\\031\'\n"
	".. \']\'\n"
	"local function json_string_literal(value)\n"
	"local newval = value:gsub(chars_to_be_escaped_in_JSON_string, backslash_replacement_function)\n"
	"return \'\"\' .. newval .. \'\"\'\n"
	"end\n"
	"local function object_or_array(self, T, etc)\n"
	"local string_keys = { }\n"
	"local seen_number_key = false\n"
	"local maximum_number_key\n"
	"for key in pairs(T) do\n"
	"if type(key) == \'number\' then\n"
	"seen_number_key = true\n"
	"if not maximum_number_key or maximum_number_key < key then\n"
	"maximum_number_key = key\n"
	"end\n"
	"elseif type(key) == \'string\' then\n"
	"table.insert(string_keys, key)\n"
	"else\n"
	"self:onEncodeError(\"can\'t encode table with a key of type \" .. type(key), etc)\n"
	"end\n"
	"end\n"
	"if seen_number_key and #string_keys > 0 then\n"
	"self:onEncodeError(\"a table with both numeric and string keys could be an object or array; aborting\", etc)\n"
	"elseif #string_keys == 0  then\n"
	"if seen_number_key then\n"
	"return nil, maximum_number_key\n"
	"else\n"
	"if tostring(T) == \"JSON array\" then\n"
	"return nil\n"
	"elseif tostring(T) == \"JSON object\" then\n"
	"return { }\n"
	"else\n"
	"return nil\n"
	"end\n"
	"end\n"
	"else\n"
	"table.sort(string_keys)\n"
	"return string_keys\n"
	"end\n"
	"end\n"
	"local encode_value\n"
	"function encode_value(self, value, parents, etc)\n"
	"if value == nil then\n"
	"return \'null\'\n"
	"end\n"
	"if type(value) == \'string\' then\n"
	"return json_string_literal(value)\n"
	"elseif type(value) == \'number\' then\n"
	"if value ~= value then\n"
	"return \"null\"\n"
	"elseif value >= math.huge then\n"
	"return \"1e+9999\"\n"
	"elseif value <= -math.huge then\n"
	"return \"-1e+9999\"\n"
	"else\n"
	"return tostring(value)\n"
	"end\n"
	"elseif type(value) == \'boolean\' then\n"
	"return tostring(value)\n"
	"elseif type(value) ~= \'table\' then\n"
	"self:onEncodeError(\"can\'t convert \" .. type(value) .. \" to JSON\", etc)\n"
	"else\n"
	"local T = value\n"
	"if parents[T] then\n"
	"self:onEncodeError(\"table \" .. tostring(T) .. \" is a child of itself\", etc)\n"
	"else\n"
	"parents[T] = true\n"
	"end\n"
	"local result_value\n"
	"local object_keys, maximum_number_key = object_or_array(self, T, etc)\n"
	"if maximum_number_key then\n"
	"local ITEMS = { }\n"
	"for i = 1, maximum_number_key do\n"
	"table.insert(ITEMS, encode_value(self, T[i], parents, etc))\n"
	"end\n"
	"result_value = \"[\" .. table.concat(ITEMS, \",\") .. \"]\"\n"
	"elseif object_keys then\n"
	"local PARTS = { }\n"
	"for _, key in ipairs(object_keys) do\n"
	"local encoded_key = encode_value(self, tostring(key), parents, etc)\n"
	"local encoded_val = encode_value(self, T[key],        parents, etc)\n"
	"table.insert(PARTS, string.format(\"%s:%s\", encoded_key, encoded_val))\n"
	"end\n"
	"result_value = \"{\" .. table.concat(PARTS, \",\") .. \"}\"\n"
	"else\n"
	"result_value = \"[]\"\n"
	"end\n"
	"parents[T] = false\n"
	"return result_value\n"
	"end\n"
	"end\n"
	"local encode_pretty_value\n"
	"function encode_pretty_value(self, value, parents, indent, etc)\n"
	"if type(value) == \'string\' then\n"
	"return json_string_literal(value)\n"
	"elseif type(value) == \'number\' then\n"
	"return tostring(value)\n"
	"elseif type(value) == \'boolean\' then\n"
	"return tostring(value)\n"
	"elseif type(value) == \'nil\' then\n"
	"return \'null\'\n"
	"elseif type(value) ~= \'table\' then\n"
	"self:onEncodeError(\"can\'t convert \" .. type(value) .. \" to JSON\", etc)\n"
	"else\n"
	"local T = value\n"
	"if parents[T] then\n"
	"self:onEncodeError(\"table \" .. tostring(T) .. \" is a child of itself\", etc)\n"
	"end\n"
	"parents[T] = true\n"
	"local result_value\n"
	"local object_keys = object_or_array(self, T, etc)\n"
	"if not object_keys then\n"
	"local ITEMS = { } for i = 1, #T do\n"
	"table.insert(ITEMS, encode_pretty_value(self, T[i], parents, indent, etc))\n"
	"end result_value = \"[ \" .. table.concat(ITEMS, \", \") .. \" ]\"\n"
	"else local KEYS = { } local max_key_length = 0\n"
	"for _, key in ipairs(object_keys) do\n"
	"local encoded = encode_pretty_value(self, tostring(key), parents, \"\", etc)\n"
	"max_key_length = math.max(max_key_length, #encoded)\n"
	"table.insert(KEYS, encoded) end\n"
	"local key_indent = indent .. \"    \"\n"
	"local subtable_indent = indent .. string.rep(\" \", max_key_length + 2 + 4)\n"
	"local FORMAT = \"%s%\" .. tostring(max_key_length) .. \"s: %s\"\n"
	"local COMBINED_PARTS = { }\n"
	"for i, key in ipairs(object_keys) do\n"
	"local encoded_val = encode_pretty_value(self, T[key], parents, subtable_indent, etc)\n"
	"table.insert(COMBINED_PARTS, string.format(FORMAT, key_indent, KEYS[i], encoded_val))\n"
	"end\n"
	"result_value = \"{\\n\" .. table.concat(COMBINED_PARTS, \",\\n\") .. \"\\n\" .. indent .. \"}\"\n"
	"end parents[T] = false return result_value end end\n"
	"function OBJDEF:encode(value, etc)\n"
	"if type(self) ~= \'table\' or self.__index ~= OBJDEF then\n"
	"OBJDEF:onEncodeError(\"JSON:encode must be called in method format\", etc) end\n"
	"local parents = {}\n"
	"return encode_value(self, value, parents, etc) end\n"
	"function OBJDEF:encode_pretty(value, etc)\n"
	"local parents = {}\n"
	"local subtable_indent = \"\"\n"
	"return encode_pretty_value(self, value, parents, subtable_indent, etc)\n"
	"end\n"
	"function OBJDEF.__tostring()\n"
	"return \"JSON encode/decode package\"\n"
	"end\n"
	"OBJDEF.__index = OBJDEF\n"
	"function OBJDEF:new(args)\n"
	"local new = { }\n"
	"if args then\n"
	"for key, val in pairs(args) do\n"
	"new[key] = val\n"
	"end end\n"
	"return setmetatable(new, OBJDEF)\n"
	"end\n"
	"return OBJDEF:new()\n"
	"\nend)()\n"
	"json={"
	"enc=function(dat) return _JSON:encode(dat) end,"
	"dec=function(dat) return _JSON:decode(dat) end}";
