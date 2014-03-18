
--[[
-- this heap sort is implemented from the heap sort algorithm 
-- in <<Introduction to algorithm>> written by Cormen etc.
--]]

local heap_size = 0
local cache_len = 0
local cache = {}


function LEFT(i)
    return i * 2
end

function RIGHT(i)
    return i * 2 + 1
end

function PARENT(i)
    return math.floor(i / 2)
end

function exchange(i, smallest)
    local t
    t = cache[i]
    cache[i] = cache[smallest]
    cache[smallest] = t
end

function min_heapify(i)
    local l = LEFT(i)
    local r = RIGHT(i)
    local smallest

    if l <= heap_size and cache[l].score < cache[i].score then
        smallest = l
    else 
        smallest = i
    end

    if r <= heap_size and cache[r].score < cache[smallest].score then          
        smallest = r
    end

    if smallest ~= i then
        -- exchange cache[i] and cache[smallest]
        exchange(i, smallest)
        
        -- recursively execute min_heapify
        min_heapify(smallest)
    end
end

function min_build_heap()
    local cl = math.floor(cache_len / 2)

    for i = cl, 1, -1 do
        min_heapify(i)
    end
end

function heapsort(c, s)
    heap_size = s
    cache_len = s
    cache = c
    
    min_build_heap()
    for i = cache_len, 2, -1 do
        exchange(1, i)
        heap_size = heap_size - 1
        min_heapify(i)
    end

    return cache
end

