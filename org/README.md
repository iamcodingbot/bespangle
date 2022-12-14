# org smart contract

This smart contract allows an org to produce and to consume badges.


## Producers and Consumers

Early dive into these two broad categories makes for easier understanding.

### Producer Badge Contracts
Producers Badge Contracts produce the badges. Two Producer Contracts are available -
- Simple Badge Contract - Creates a badge, which a person of authority in an org can issue to  members of org.
- Gotcha Badge Contact - Creates a badge which members of org can issue to other members. A gotcha badge comes with Supply Cap for a Time Duration Cycle. Supply Cap is restored at end of every Time Duration Cycle.

### Consumer Badge Contracts
Three Consumers Contracts are available -
- Atomic Assets Consumer - This consumes badges at granular level in atomic asset standard. i.e. for every Badge a non transferrable asset is created in atomic assets standard. 
- Cumulative Consumer - This consumes badges at cumulative level for an account. Same badges are added and stored as one record.
- Rounds Consumer - This consumer accumulate badges and their corresponding scores for an account from the start to the end of the round. Start and End of round are triggered by calling actions.

## Usage

For org to use above features, it need to -
### 1 -  Deploy org smart contract and validation contract.
Deploy this contract and a validation contract similar to Eden Validation Contract.

### 2 - Initialize org contract

Org would need to do some basic setup, which involves
- Subscribing to the producer contracts.
- Subscribing to a validation contract. Org needs to implement this to validate the input of actions. See Eden Validation Contract.
- Optionally give atomic asset collection name if org wishes to subscribe to atomic asset consumer.

All this can be done via this action -
```    
ACTION initsystem(name checks_contract, 
      vector<name> producers, 
      name aacollection);

      checks_contract - org authored contract to validate input fields of various actions. 
                        Mainly this can be used to check authority of action execution.
      producers - vector of producer contracts like simple badge contract and gotcha badge contract
      aacollection - atomic asset collection name
```

### 3 - Badge definition 
With initial setup out of the way, next an org can define some simple badges and gotcha badges using below actions.
```
ACTION initsimple (name creator, 
      name badge, 
      string ipfs_image, 
      string display_name, 
      vector<name> consumers);

      creator - account creating simple badge. Use checks_contract to validate the authority.
      badge - name of the badge
      ipfs_image -ipfs hash of the inage
      display_name- text for display purpose.
      consumers - vector of consumers this badge has subscribed to.
```


```
ACTION initgotcha (name creator, 
      name badge, 
      time_point_sec starttime, 
      uint64_t cycle_length, 
      uint8_t supply_per_cycle, 
      string ipfs_image, 
      string display_name, 
      vector<name> consumers);

      creator - account creating gotcha badge. Use checks_contract to validate the authority.
      badge - name of the badge.
      starttime - time_point_sec at which members can start issuing badges.
      cycle_length - Duration in seconds after supply_per_cycle is restored.
      supply_per_cycle - supply available to issue per cycle_length
      display_name - text for display purpose.
      consumers - vector of consumers this badge has subscribed to.
```

### 4 - Badge Issuance
With badge definition done, members and issuing authority can start issue badges via following actions.
```
ACTION givesimple (name badge, name authorizer, name to, string memo );

      badge - name of simple badge to be issued.
      authorizer - authorized account to issue badge. Use checks_contract contract to validate the authority.
      to - beneficiary of the badge.
      memo - text field to optionally specify reason for issuance.
```

```
ACTION givegotcha (name badge, name from, name to, uint8_t amount, string memo );

      badge - name of gotcha badge to be issued.
      from - issuer
      to - beneficiary
      amount - amount
      memo - text field to optionally specify reason for issuance.

```
### 5 - Rounds consumer setup

For badge Subscribing to rounds consumer, following additional steps are needed - 

#### 1) create a round
Creates a new round in init status.
```
ACTION initround (name authorizor, name round, string display_name);
```

#### 2) add a badge and add scoring method.
```
ACTION addbdgetornd (name authorizer,
      name round, 
      uint64_t badge_id, 
      name balance_based_scoring_type, 
      uint16_t balance_based_scoring_weight,
      name source_based_scoring_type,
      uint16_t source_based_scoring_weight);

      authorizer - authorized account to add a badge for scoring.
      round - name of the round
      balance_based_scoring_type - linear or fibonacci.
      balance_based_scoring_weight - wt of balance_based_scoring_type
      source_based_scoring_type - linear or fibonacci
      source_based_scoring_weight - wt of source_based_scoring_weight
      
```

#### 3) start a round
Start a round by calling this action.
```
ACTION startround (name authorizer, name round);
```

#### 4) end a round
End a round by calling this action.
```
ACTION endround (name authorizer, name round);
```


For accounts to interact with badges following actions are available in userprefs smart contract.

#### 1 - Opt in to receive badges
```
ACTION accntoptin(name account);

      account - account opting in to receive badges from org.
    
```

#### 2 - opt out 
```
ACTION accntoptout(name account, string memo);

      account - account opting out of system.
```



