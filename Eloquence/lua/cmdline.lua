_G._repl = require("console")

function _repl:displayresults(results)
	if results.n == 0 then
		return
	end

	print(table.unpack(results, 1, results.n))
end

function _repl:displayerror(err)
	print(err)
end

-- Copyright (c) 2011-2012 Rob Hoelz <rob@hoelz.ro>
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy of
-- this software and associated documentation files (the "Software"), to deal in
-- the Software without restriction, including without limitation the rights to
-- use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
-- the Software, and to permit persons to whom the Software is furnished to do so,
-- subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
-- FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
-- COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
-- IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
-- CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

-- Pretty prints expression results (console only)

local format = string.format
local tsort = table.sort
local tostring = tostring
local type = type
local floor = math.floor
local pairs = pairs
local ipairs = ipairs
local error = error
local stderr = io.stderr

pcall(require, 'luarocks.require')
local ok, term = pcall(require, 'term')
if not ok then
  term = nil
end

local keywords = {
  ['and'] = true,
  ['break'] = true,
  ['do'] = true,
  ['else'] = true,
  ['elseif'] = true,
  ['end'] = true,
  ['false'] = true,
  ['for'] = true,
  ['function'] = true,
  ['if'] = true,
  ['in'] = true,
  ['local'] = true,
  ['nil'] = true,
  ['not'] = true,
  ['or'] = true,
  ['repeat'] = true,
  ['return'] = true,
  ['then'] = true,
  ['true'] = true,
  ['until'] = true,
  ['while'] = true,
}

local function compose(f, g)
  return function(...)
    return f(g(...))
  end
end

local emptycolormap = setmetatable({}, { __index = function()
  return function(s)
    return s
  end
end})

local colormap = emptycolormap

if term then
  colormap = {
    ['nil'] = term.colors.blue,
    string = term.colors.yellow,
    punctuation = compose(term.colors.green, term.colors.bright),
    ident = term.colors.red,
    boolean = term.colors.green,
    number = term.colors.cyan,
    path = term.colors.white,
  }
end

local function isinteger(n)
  return type(n) == 'number' and floor(n) == n
end

local function isident(s)
  return type(s) == 'string' and not keywords[s] and s:match('^[a-zA-Z_][a-zA-Z0-9_]*$')
end

local function sortedpairs(t)
  local keys = {}

  for k in pairs(t) do
    keys[#keys + 1] = k
  end
  tsort(keys)

  local index = 1
  return function()
    if keys[index] == nil then
      return nil
    else
      local key = keys[index]
      local value = t[key]
      index = index + 1

      return key, value
    end
  end, keys
end

local function find_longstring_nest_level(s)
  local level = 0

  while s:find(']' .. string.rep('=', level) .. ']', 1, true) do
    level = level + 1
  end

  return level
end

local function dump(seen, path, v, indent)
  local t = type(v)

  if t == 'nil' or t == 'boolean' or t == 'number' then
    stderr:write(colormap[t](tostring(v)))
  elseif t == 'string' then
    stderr:write(tostring(v))
  elseif t == 'table' then
    if seen[v] then
      stderr:write(colormap.path(seen[v]))
      return
    end

    seen[v] = path

    local lastintkey = 0

    stderr:write(colormap.punctuation '{\n')
    for i, v in ipairs(v) do
      for j = 1, indent do
        stderr:write ' '
      end
      dump(seen, path .. '[' .. tostring(i) .. ']', v, indent + 1)
      stderr:write(colormap.punctuation ',\n')
      lastintkey = i
    end

    for k, v in sortedpairs(v) do
      if not (isinteger(k) and k <= lastintkey and k > 0) then
        for j = 1, indent do
          stderr:write ' '
        end

        if isident(k) then
          stderr:write(colormap.ident(k))
        else
          stderr:write(colormap.punctuation '[')
          dump(seen, path .. '.' .. tostring(k), k, indent + 1)
          stderr:write(colormap.punctuation ']')
        end
        stderr:write(colormap.punctuation ' = ')
        dump(seen, path .. '.' .. tostring(k), v, indent + 1)
        stderr:write(colormap.punctuation ',\n')
      end
    end

    for j = 1, indent - 1 do
      stderr:write ' '
    end

    stderr:write(colormap.punctuation '}')
  elseif t == 'function' then
  	stderr:write(tostring(v))
  else
    error(format('Cannot print type \'%s\'', t))
  end
end

function _repl:displayresults(results)
  for i = 1, results.n do
    dump({}, '<topvalue>', results[i], 1)
    stderr:write '\n'
  end
end
