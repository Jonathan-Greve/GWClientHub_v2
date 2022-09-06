#pragma once


/**
 * \brief Adds various playerstate to the redis pipeline such as: position, health, energy, skillbar etc.
 * For performance it might only add some things to the pipeline when they have changed. For example
 * the questlog rarely changes and contains long strings, so we will only update those rarely. For state
 * like position we might update it on every call independent of whether it changed or not as it is likely
 * to change often. To see the specific conditions about when something is added to the pipeline you must 
 * read the source code.
 * \param dt Time since last call in milliseconds.
 */
void AddPlayerStateToRedisPipe(float dt);
