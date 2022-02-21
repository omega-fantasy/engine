local total_profit = 0
local towns = towns()
for t_index, town in pairs(towns) do
    local buildings = buildings(town)
    for b_index, building in pairs(buildings) do
        if property_exists(building, "GenerateMoney") ~= 0 then
            local profit = property_get(building, "GenerateMoney")
            total_profit = total_profit + profit
        else
            total_profit = total_profit - 5  
        end
    end
end

player_money_change(total_profit)
print("Player money was increased by: ", total_profit)