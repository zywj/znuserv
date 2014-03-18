
require('src.zs_heapsort')

--[[
cache[uri] = page

elements in each page:
	[index] is use to quick sort 
	[score] refers to the points of this page
	[time] refers to the time the page be visited most recently  
	[p] refers to the counters the page be visited
	[content] refers to the content of the this page 
	[modified_time] refers to the page be modified time
--]]
local cache = {}
local index = 0
local page_count = 0


function init(pc)
	page_count = pc
end

function is_in_cache(key)
	if cache[key] == nil or cache[key].content == nil then
		return 0
	else
		return 1
	end
end

function sync_content(key, content, md)

	-- if the modified time isn't same as md, the content must be refreshed
	if cache[key].modified_time ~= md then
		cache[key].content = content
	end
end

-- store the html file content into the cache
function store_cache(key, content, md)
	local page = {}

    page.p = 1
    page.score = 1
    page.time = os.time()
	page.content = content
	page.modified_time = md

	cache[key] = page	
	index = index + 1  
	cache[index] = cache[key]
end

-- recalculate all scores 
function recalc_all_scores()
	for key in pairs(cache) do
		if type(key) == "string" then
			local now_time, delta_time, delta_alive_time	

			-- get the time delta
			now_time = os.time()
			delta_time = (now_time - cache[key].time)  / 3600

			-- recalculate the score
			cache[key].score = cache[key].p / (delta_time + 1) ^ 1.8
		end
	end

	-- then resort every page, generate a new rank for top N pages
	if index > 1 then 
		cache = heapsort(cache, index) 

		-- set the page content to nil if the page is out of top N
		for i = page_count, index do
			cache[i].content = nil
		end
	end
end

function reset_p()
	for i = 1, index do
		cache[key].p = index - i + 1
	end
end

-- return the request html file content
function get_cache(key, content, md)
    local cnt

    if is_in_cache(key) == 1 then

		-- if the page is in the cache, but the content had been reset nil
		-- i.e. active the page
		if cache[key] ~= nil and cache[key].content == nil then
			cache[key].content = content
		else 
	    	-- before get cache content, it is need to sync the content,
	    	-- because the content mybe modified.
	    	sync_content(key, content, md)

	    	-- cnt is the newest content 
		    cnt = cache[key].content

		    -- if the p is too big, it needs to reset
		    if cache[key].p == math.huge then
		    	reset_p()
		    else
		    	cache[key].p = cache[key].p + 1
		    end

		    -- all score of page will be recalculated when any page is visited.
		    recalc_all_scores()

		    print("** get page from cache **")
		    return cnt
		end
	else 
		print("in get cache, no page in")
		return -1
	end
end
