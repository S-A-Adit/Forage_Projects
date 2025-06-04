#include <string>
#include <iostream>
#include <utility> // For std::move
#include <vector>  // For std::vector
#include <memory>  // For std::unique_ptr
#include <limits>  // For std::numeric_limits, used with std::cin.ignore
#include <algorithm> // For std::find_if, or std::remove_if if used

class Item {
private:
    std::string name;
    int quantity;
    float price;

public:
    Item(
        std::string name,
        int quantity,
        float price
    ) :
        name{std::move(name)}, // Use std::move for efficiency with string
        quantity{quantity},
        price{price} {
        // Constructor body can be empty if using initializer list
    }

    // Destructor to confirm item destruction (optional, for debugging)
    ~Item() {
        // std::cout << "DEBUG: Item '" << name << "' destroyed." << std::endl;
    }

    std::string get_name() const {
        return name;
    }

    int get_quantity() const {
        return quantity;
    }

    void set_quantity(int new_quantity) {
        quantity = new_quantity;
    }

    float get_price() const {
        return price;
    }

    // Changed to const reference for efficiency and correctness
    bool is_match(const std::string &other_name) const {
        return name == other_name;
    }
};

class Inventory {
private:
    // Use std::vector to store unique_ptrs to Item objects
    std::vector<std::unique_ptr<Item>> items;
    float total_money;
    // item_count is no longer needed; items.size() provides it

    // Changed to take const reference to Item for efficiency
    static void display_data(const Item &item) {
        std::cout << "\nItem name: " << item.get_name();
        std::cout << "\nQuantity: " << item.get_quantity();
        std::cout << "\nPrice: " << item.get_price();
    }

public:
    Inventory() :
        total_money{0} { // items is default-constructed (empty vector)
    }

    // Rule of Five: If you manage raw pointers/resources, you need custom
    // copy constructor, copy assignment, move constructor, move assignment, and destructor.
    // By using unique_ptr, the default move operations are fine, and copy operations are deleted.
    // Destructor of Inventory will automatically destroy all unique_ptrs, which in turn
    // destroy the Item objects. No manual loop needed for deletion.

    // A better way to add item: accept data, or a unique_ptr
    void add_item() {
        std::string name;
        int quantity;
        float price;

        // Clear input buffer before reading string
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "\nEnter item name: ";
        std::getline(std::cin, name); // Use getline for names with spaces
        
        std::cout << "Enter quantity: ";
        while (!(std::cin >> quantity) || quantity <= 0) {
            std::cout << "Invalid quantity. Please enter a positive number: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::cout << "Enter price: ";
        while (!(std::cin >> price) || price < 0) {
            std::cout << "Invalid price. Please enter a non-negative number: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        
        // After reading price, there might be a newline left in the buffer.
        // It's good practice to clear it again if you expect another getline soon.
        // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


        // Check if item already exists to increment quantity instead of adding new
        auto it = std::find_if(items.begin(), items.end(),
                               [&name](const std::unique_ptr<Item>& item_ptr) {
                                   return item_ptr->is_match(name);
                               });

        if (it != items.end()) {
            // Item found, just update quantity
            (*it)->set_quantity((*it)->get_quantity() + quantity);
            std::cout << "\nItem '" << name << "' already exists. Quantity updated." << std::endl;
        } else {
            // Item not found, add new
            items.push_back(std::make_unique<Item>(name, quantity, price));
            std::cout << "\nNew item '" << name << "' added to inventory." << std::endl;
        }
    }


    void sell_item() {
        if (items.empty()) {
            std::cout << "\nInventory is empty. Nothing to sell." << std::endl;
            return;
        }

        std::string item_to_check;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer
        std::cout << "\nEnter item name to sell: ";
        std::getline(std::cin, item_to_check); // Use getline for names with spaces

        // Find the item using an iterator
        auto it = std::find_if(items.begin(), items.end(),
                               [&item_to_check](const std::unique_ptr<Item>& item_ptr) {
                                   return item_ptr->is_match(item_to_check);
                               });

        if (it != items.end()) {
            // Item found, pass the index or iterator to remove_item
            // Passing iterator is more C++ idiomatic and potentially more efficient for vector::erase
            remove_item(it);
        } else {
            std::cout << "\nThis item is not in your Inventory." << std::endl;
        }
    }

    // Modified remove_item to take an iterator
    void remove_item(std::vector<std::unique_ptr<Item>>::iterator item_it) {
        int input_quantity;
        Item *item = item_it->get(); // Get raw pointer from unique_ptr for convenience

        std::cout << "\nEnter number of items to sell: ";
        while (!(std::cin >> input_quantity) || input_quantity <= 0) {
            std::cout << "Invalid quantity. Please enter a positive number: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        int current_quantity = item->get_quantity();

        if (input_quantity <= current_quantity) {
            float price = item->get_price();
            float money_earned = price * input_quantity;

            item->set_quantity(current_quantity - input_quantity);
            std::cout << "\nItems sold.";
            std::cout << "\nMoney received: " << money_earned;
            total_money += money_earned;

            // Check if quantity reached zero after decrementing
            if (item->get_quantity() == 0) {
                std::cout << "\nItem '" << item->get_name() << "' quantity reached zero. Removing completely." << std::endl;
                // No manual delete needed! unique_ptr handles it.
                items.erase(item_it); // Remove the unique_ptr from the vector
            }

        } else { // input_quantity > current_quantity
            std::cout << "\nCannot sell more items than you have (Current: " << current_quantity << ")." << std::endl;
        }
    }

    void list_items() {
        if (items.empty()) { // Use .empty() instead of checking item_count
            std::cout << "\nInventory empty." << std::endl;
            return;
        }

        std::cout << "\n--- Current Inventory ---" << std::endl;
        for (const auto& item_ptr : items) { // Use range-based for loop with const reference
            display_data(*item_ptr); // Dereference unique_ptr to pass Item to display_data
            std::cout << "\n";
        }
        std::cout << "Total Money: " << total_money << std::endl;
        std::cout << "-------------------------" << std::endl;
    }
};

int main() {
    int choice;
    Inventory inventory_system;
    std::cout << "Welcome to the inventory!";

    while (true) { // Use 'true' for infinite loop, clearer than '1'
        std::cout << "\n\nMENU\n"
                  << "1. Add new item\n"
                  << "2. Sell item\n"
                  << "3. List items\n"
                  << "4. Exit\n\n"
                  << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                inventory_system.add_item();
                break;

            case 2:
                inventory_system.sell_item();
                break;

            case 3:
                inventory_system.list_items();
                break;

            case 4:
                return 0; // Use return 0 instead of exit(0) for clean exit

            default:
                std::cout << "\nInvalid choice entered";
                // Clear error flags and ignore remaining input in the line
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;
        }
    }
}