// game_combat_system.h
#ifndef GAME_COMBAT_SYSTEM_H
#define GAME_COMBAT_SYSTEM_H

#include <string>
#include <vector>
#include <memory> // For std::unique_ptr or std::shared_ptr if we want ownership

// Forward declarations to avoid circular dependencies for pointers/references
class HealthComponent;
class ManaComponent;
class Ability;
class DamageCalculator;
class TargetSelection;

// --- Character Class ---
class Character {
public:
    std::string name;
    int level;
    
    // Using smart pointers for components for ownership and lifetime management
    std::unique_ptr<HealthComponent> health;
    std::unique_ptr<ManaComponent> mana;
    
    std::vector<std::unique_ptr<Ability>> abilities; // Character owns its abilities

    Character(const std::string& name, int level);
    ~Character(); // Destructor to properly clean up unique_ptrs if needed

    void attack(Character& target);
    void useAbility(const std::string& abilityName, Character& target); // Using string for simplicity to find ability
    // Or, more robust: void useAbility(Ability& ability, Character& target);
    
private:
    // Helper to find ability by name, could be made public or part of AbilityManager
    Ability* findAbility(const std::string& abilityName);
};

// --- Health Components ---

class HealthComponent {
public:
    virtual ~HealthComponent() = default; // Virtual destructor for proper polymorphic cleanup

    int currentHealth;
    int maxHealth;

    // Pure virtual functions, must be implemented by subclasses
    virtual void takeDamage(int amount) = 0;
    virtual void heal(int amount);
    virtual bool isAlive() const;

protected:
    HealthComponent(int maxHp);
};

class StandardHealth : public HealthComponent {
public:
    StandardHealth(int maxHp);
    void takeDamage(int amount) override;
};

class ArmoredHealth : public HealthComponent {
public:
    ArmoredHealth(int maxHp);
    void takeDamage(int amount) override; // May apply damage reduction
};

// --- Mana/Resource Components ---

class ManaComponent {
public:
    virtual ~ManaComponent() = default; // Virtual destructor

    int currentMana;
    int maxMana;

    // Pure virtual functions
    virtual bool consumeMana(int amount) = 0;
    virtual void regenerateMana(int amount) = 0;

protected:
    ManaComponent(int maxResource);
};

class ArcaneMana : public ManaComponent {
public:
    ArcaneMana(int maxResource);
    bool consumeMana(int amount) override;
    void regenerateMana(int amount) override;
};

class RageEnergy : public ManaComponent {
public:
    RageEnergy(int maxResource);
    bool consumeMana(int amount) override; // Renamed for clarity in diagram, but still consumeMana here
    void regenerateMana(int amount) override; // Might be triggered by combat
};

// --- Ability Classes ---

class Ability {
public:
    virtual ~Ability() = default; // Virtual destructor

    std::string name;
    int resourceCost;
    int cooldown; // Not directly used in methods here, but good to include

    // Pure virtual function
    virtual void activate(Character& caster, Character& target) = 0;

protected:
    // Pointers to collaborators, assuming they are managed externally or passed by ref
    DamageCalculator* damageCalculator;
    TargetSelection* targetSelection;

    Ability(const std::string& name, int cost, DamageCalculator* dc, TargetSelection* ts);
};

class MeleeAttack : public Ability {
public:
    int baseDamage;
    MeleeAttack(int baseDmg, DamageCalculator* dc, TargetSelection* ts);
    void activate(Character& caster, Character& target) override;
};

class SpellCast : public Ability {
public:
    std::string spellEffect; // Could be an enum or a more complex effect object
    SpellCast(const std::string& effect, DamageCalculator* dc, TargetSelection* ts);
    void activate(Character& caster, Character& target) override;
};

class Buff : public Ability {
public:
    std::string effectDescription;
    int duration;
    Buff(const std::string& description, int dur);
    void activate(Character& caster, Character& target) override;
    void apply(Character& target); // Helper to apply the actual effect
    void remove(Character& target); // Helper to remove the effect
};

class Debuff : public Ability {
public:
    std::string effectDescription;
    int duration;
    Debuff(const std::string& description, int dur);
    void activate(Character& caster, Character& target) override;
    void apply(Character& target); // Helper to apply the actual effect
    void remove(Character& target); // Helper to remove the effect
};

// --- Utility Classes ---

class DamageCalculator {
public:
    int calculateDamage(const Ability& ability, const Character& caster, const Character& target);
    // Might have more complex parameters like armor, resistances, etc.
};

class TargetSelection {
public:
    // Returns a vector of pointers/references to characters that are valid targets
    std::vector<Character*> selectTargets(Character& caster, const Ability& ability);
    // Overload for specific targeting types (e.g., single, area, self)
    std::vector<Character*> selectTargets(Character& caster, const Ability& ability, const std::string& targetType);
};

#endif // GAME_COMBAT_SYSTEM_H