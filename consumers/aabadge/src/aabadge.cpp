#include <aabadge.hpp>

ACTION aabadge::initcoll (name org, name collection_name) {
  require_auth( org );
  aacollection_table _aacollection ( _self, _self.value);
  auto itr = _aacollection.find (org.value);
  check (itr == _aacollection.end(), "<org> already has a <collection_name> allocated");
  _aacollection.emplace(get_self(), [&](auto& row){
    row.org = org;
    row.collection = collection_name;
  });

  vector<name> authorized_accounts ;
  authorized_accounts.push_back(get_self());

  vector<name> notify_accounts ;
  notify_accounts.push_back(get_self());

  std::map <std::string, ATOMIC_ATTRIBUTE> data;
  
  action {
    permission_level{get_self(), name("active")},
    name(ATOMIC_ASSETS_CONTRACT),
    name("createcol"),
    createcol_args {
      .author = get_self(),
      .collection_name = collection_name,
      .allow_notify = true,
      .authorized_accounts = authorized_accounts,
      .notify_accounts = notify_accounts,
      .market_fee = 0.00,
      .data = data}
  }.send();

  vector <FORMAT> schema_format;
  schema_format.push_back(FORMAT{.name = "name", .type = "string"});
  schema_format.push_back(FORMAT{.name = "contract",.type = "string"});
  schema_format.push_back(FORMAT{.name = "badge",.type = "string"});
  schema_format.push_back(FORMAT{.name = "badge_id",.type = "string"});
  schema_format.push_back(FORMAT{.name = "img",.type = "string"});
  

  action {
    permission_level{get_self(), name("active")},
    name(ATOMIC_ASSETS_CONTRACT),
    name("createschema"),
    createschema_args {
      .authorized_creator = get_self(),
      .collection_name = collection_name,
      .schema_name = name(ATOMIC_ASSETS_SCHEMA_NAME),
      .schema_format = schema_format}
  }.send();
}

void aabadge::notifyinit(
    name org,
    name badge_contract,
    name badge_name,
    name notify_account,
    string memo, 
    uint64_t badge_id, 
    vector<ipfs_hash> ipfs_hashes,
    uint32_t rarity_counts) {
      
  ATTRIBUTE_MAP mdata = {};
  aacollection_table _aacollection (_self, _self.value);
  auto itr = _aacollection.find(org.value); 
  check (itr != _aacollection.end(), "Initialize a collection for your org using initcoll action.");
  name collection = itr->collection;
  string ipfs_image;
  for( auto i = 0; i < ipfs_hashes.size(); i++) {
    // todo? can img tag not exist?
    if (ipfs_hashes[i].key == "img") {
      ipfs_image = ipfs_hashes[i].value;
    }
  }

  mdata["badge"] = string(badge_name.to_string());
  mdata["badge_id"] = to_string(badge_id);
  mdata["contract"] = string(badge_contract.to_string());
  mdata["img"] = ipfs_image;
  mdata["name"] = memo;

  // todo? check if template already exists, update mdata then
  // have a notifyupdate
  aatemplate_table _aatemplate (_self, org.value);
  auto aatemplate_itr = _aatemplate.find( badge_id );
  if (aatemplate_itr == _aatemplate.end()) {

    _aatemplate.emplace(get_self(), [&](auto& row){
      row.badge_id = badge_id;
    });

    action {
    permission_level{get_self(), name("active")},
    name(ATOMIC_ASSETS_CONTRACT),
    name("createtempl"),
    createtemplate_args {
      .authorized_creator = get_self(),
      .collection_name = collection,
      .schema_name = name(ATOMIC_ASSETS_SCHEMA_NAME),
      .transferable = false,
      .burnable = false,
      .max_supply = 0,
      .immutable_data = mdata}
    }.send();
  } 
}

void aabadge::updatebadge(
    int32_t template_id,
    name authorized_creator,
    name collection_name,
    name schema_name,
    bool transferable,
    bool burnable,
    uint32_t max_supply,
    ATTRIBUTE_MAP immutable_data) {
  aacollection_table _aacollection ( _self, _self.value);
  auto collection_index = _aacollection.get_index<name("colkey")>();
  auto collection_iterator = collection_index.require_find (collection_name.value, "somethings wrong, collection not found");
  name org = collection_iterator->org;
  
  uint64_t badge_id = std::stoull(get<string>(immutable_data["badge_id"]));

  aatemplate_table _aatemplate (_self, org.value);
  auto itr = _aatemplate.require_find(badge_id, "somethings wrong , not found");
  _aatemplate.modify(itr, get_self(), [&](auto& row){
    row.template_id = template_id;
  });
}

void aabadge::notifyachiev (name org, 
    name badge_contract, 
    name badge_name,
    name account, 
    name from,
    uint8_t count,
    string memo,
    uint64_t badge_id,  
    vector<name> notify_accounts) {

  aacollection_table _aacollection (_self, _self.value);
  auto itr = _aacollection.find(org.value); 
  check (itr != _aacollection.end(), "Initialize a collection for your org initcoll action. Params needed org name and choice of collection name ");
  name collection = itr->collection;

  aatemplate_table _aatemplate (_self, org.value);
  auto template_itr = _aatemplate.require_find(badge_id, "somethings wrong , not found");

  std::map <std::string, ATOMIC_ATTRIBUTE> immutable_data;
  std::map <std::string, ATOMIC_ATTRIBUTE> mutable_data;
  vector<asset> tokens_to_back;
  for (auto i = 0; i < count; i++) {
    action {
      permission_level{get_self(), name("active")},
      name(ATOMIC_ASSETS_CONTRACT),
      name("mintasset"),
      mintaa_args {
        .authorized_creator = get_self(),
        .collection_name = collection,
        .schema_name = name(ATOMIC_ASSETS_SCHEMA_NAME),
        .template_id = template_itr->template_id,
        .new_asset_owner = account,
        .immutable_data = immutable_data,
        .mutable_data = mutable_data,
        .tokens_to_back = tokens_to_back}
    }.send();
  }
}