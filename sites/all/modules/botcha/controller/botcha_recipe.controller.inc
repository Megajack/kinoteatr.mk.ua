<?php

/*
 * Controller layer of the BotchaRecipe objects.
 */
/**
 * Abstract class to describe recipe data structure.
 */
abstract class BotchaRecipe {
  /**
   * Identifier of recipe.
   */
  public $id;

  /**
   * Brief description of the recipe.
   * It should contain explanation of how bots would fail with it
   * and what the recipe exactly does.
   */
  protected $description;

  /**
   * This parameter is used to understand what is the result of applying the recipe.
   */
  public $status;

  /**
   * Options that received as parameters turned into settings
   * by merging with default values.
   */
  protected $settings = array();

  /**
   * CSS to add to the page.
   */
  protected $css;

  /**
   * Javascript to add to the page.
   */
  protected $js;

  /**
   * @todo Do we really need it? Probably the best way is to provide mail field always - it hides our fields.
   * Name of the field in the form to use in error messages
   * (to mask botcha fields).
   */
  public $error_field;

  /**
   * Text to give users if botcha recipe blocks submission.
   * It should give some help to real human users in cases
   * of disabled Javascript or CSS.
   */
  public $error_text;

  protected $recipebooks = array();

  public static function getRecipe($id) {
    $r = BotchaRecipeModel::getRecipe($id);
    $classname = $r->classname;
    $recipe = new $classname($id);
    $recipe->setTitle($r->title)
      ->setDescription($r->description);
    return $recipe;
  }

  public function setRecipebook($rbid) {
    $this->recipebooks[$rbid] = $rbid;
    return $this;
  }

  public function save() {
    BotchaRecipeModel::save($this);
  }

  function setTitle($title) {
    $this->title = $title;
    return $this;
  }

  function getTitle() {
    return $this->title;
  }

  function setDescription($description) {
    $this->description = $description;
    return $this;
  }

  function getDescription() {
    return $this->description;
  }

  function setSecret($secret) {
    $this->secret = $secret;
    return $this;
  }

  function getSecret() {
    return $this->secret;
  }

  function setMethod($method) {
    $this->method = $method;
    return $this;
  }

  function getMethod() {
    return $this->method;
  }

  /**
   * Magic method __construct.
   */
  function __construct($id) {
    $this->id = $id;
    // Get human-readable description about this recipe
    // to clarify its work process.
    $this->getInfo();
  }

  /**
   * Used to get information about the recipe.
   * Must be overridden.
   */
  function getInfo() {
    $this->status = 'none';
    $this->error_field = 'mail';
    $this->error_text = t('You must be a human, not a spam bot, to submit forms on this website.') . ' ' .
      t('If you insist that you are a human, please try again.') . ' ' .
      t('If error persists, contact webmaster using contact link at the bottom of this page and give all the details of this error (your browser, version, OS).');
  }

  /**
   * Used to get default recipe data structure.
   */
  function getDefaultSettings() {
    return array(
      'fields' => $this->getFields(),
      'css' => $this->getCss(),
      'js' => $this->getJs(),
    );
  }

  /**
   * Universal getter.
   * Wrapper getProperty is used to let class methods be used not only in getting
   * default settings. It gives flexibility to make calls to the class methods
   * in any order: the first of them will always calculate the property value
   * and set the setting, while others will just get this already calculated value.
   * It also provides consistency: we are sure that when we get some property,
   * it is set appropriately.
   */
  function getProperty(&$value, $getter_callback, $parameters = NULL) {
    if (empty($value)) {
      $value = $this->$getter_callback($parameters);
    }
    return $value;
  }

  /**
   * Spam check.
   *
   * @param type $form
   * @param type $form_state
   */
  function isSpam($form, $form_state) {}

  /**
   * Handle form depending on the result of spam check.
   *
   * @param type $result
   * @param type $form
   * @param type $form_state
   */
  function handle($result, $form, $form_state) {
    // !!~ @todo handle Implement real logic of handling.
    switch ($result) {
      case 'success':
        break;
      case 'spam':
      default:
        break;
    }
  }

  function getSeed() {
    return md5(get_class($this) . substr($this->secret, 0, -4));
  }

  function getFields() {
    $fields_count = $this->getFieldCount();
    $fields = array();
    for ($i = 0; $i < $fields_count; $i++) {
      $fields[$i] = $this->getField($i);
    }
    return $fields;
  }

  // @todo BotchaRecipe getField Replace deltas with machine names.
  function getField($delta) {
    return array(
      'name' => $this->getProperty(
        $this->settings['fields'][$delta]['name'],
        'getFieldName',
        $delta
      ),
      'class' => $this->getProperty(
        $this->settings['fields'][$delta]['class'],
        'getFieldClass',
        $delta
      ),
      'prefix' => $this->getProperty(
        $this->settings['fields'][$delta]['prefix'],
        'getFieldPrefix',
        $delta
      ),
    );
  }

  /**
   * Should be overridden.
   *
   * @return string
   */
  public function getCss() {}

  /**
   * Should be overridden.
   *
   * @return array
   */
  public function getJs() {}

  // @todo BotchaRecipe getFieldName Replace deltas with machine names.
  function getFieldName($delta) {
    return substr($this->getProperty($this->seed, 'getSeed'), 0, 3) . '_name';
  }

  // @todo BotchaRecipe getFieldClass Replace deltas with machine names.
  function getFieldClass($delta) {
    // 'a' fix for Firefox - it breaks on ".<number>" class in CSS filter!
    return 'a' . substr($this->getProperty($this->seed, 'getSeed'), 1, 4) . '_field';
  }

  // @todo BotchaRecipe getFieldPrefix Replace deltas with machine names.
  function getFieldPrefix($delta) {
    return substr($this->getProperty($this->seed, 'getSeed'), 10, mt_rand(3, 6));
  }

  /**
   * Used to get information about the recipe.
   * Must be overridden with calling to parent::generateFormElements.
   * @todo BotchaRecipe generateFormElements Switch from indexed array to associative.
   */
  function generateFormElements() {
    $css = $this->getProperty(
      $this->settings['css'],
      'getCss'
    );
    if (!empty($css)) {
      drupal_add_css('' . $this->settings['css'] . '', array('type' => 'inline'));
    }
    return array();
  }

  /*
   * Apply recipe modifying form properties.
   */
  public function applyRecipe(&$form, &$form_state) {
    // Add BOTCHA fields to the form.
    $form_elements = $this->generateFormElements();
    foreach ($form_elements as $field_name => $field_properties) {
      if ($this->method == 'build_id_submit') {
        // Save submitted values in our stash for later use in _validate,
        // as we have to reset them here at _form_alter stage.
        // It won't be possible to reset after validation as there is no
        // reliable mechanism in Form API, i.e. form_set_value() does not
        // change rendered form and form errors disable whole 'rebuild' business.
        if (isset($_POST[$field_name])) {
          $form_state['botcha_submit_values'][$field_name] = $_POST[$field_name];
        }
        if (isset($field_properties['#default_value'])) {
          // Reset our controls to defaults here (as explained above).
          $form[$field_name]['#value'] = $field_properties['#default_value'];
          $form_state['post'][$field_name] = $field_properties['#default_value'];
          $_POST[$field_name] = $field_properties['#default_value'];
        }
      }
      else {
        unset($field_properties['!valid_token']);
        $form[$field_name] = $field_properties;
      }
    }
  }
}


class BotchaRecipeNoResubmit extends BotchaRecipe {

  function getInfo() {
    parent::getInfo();
    $this->description = t('Prevent form resubmission.'
      . ' Bots will try to resubmit old form prepared.'
      . ' Form is remembered, and only one submission is allowed.');
    $this->error_text .= '<br />' . t('Form session reuse detected.')
      . ' ' . t('An old form was submitted again, which may happen'
      . ' if it was retrieved from browser history using "Back" button.')
      . '<br />' . t('Please try again - fill all entries on this page'
      . ' without going "Back".');
  }

  function isSpam($form, $form_state) {
    // !!~ @todo Reduce code duplication.
    // @see BotchaRecipe:applyRecipe
    $isSpam = TRUE;
    $build_id = isset($_POST['form_build_id']) ? $_POST['form_build_id'] : $form['#build_id'];
    if ($cached = cache_get("botcha_$build_id", 'cache_form')) {
      $data = $cached->data;
      if (isset($data['#cache_token']) && ($data['#cache_token'] == $this->getToken())) {
        $isSpam = FALSE;
      }
    }
    return $isSpam;
  }

  protected function getToken($value = '') {
    if (empty($_SESSION['botcha_session'])) {
      $_SESSION['botcha_session'] = session_id();
    }
    return drupal_hmac_base64($value, $_SESSION['botcha_session'] . drupal_get_private_key() . drupal_get_hash_salt());
  }

  function applyRecipe(&$form, &$form_state) {
    parent::applyRecipe($form, $form_state);
    // Save build id.
    $build_id = $form['#build_id'];
    // !!~ @todo Reduce code duplication.
    // @see BotchaRecipe:applyRecipe
    $build_id_submit = isset($_POST['form_build_id']) ? $_POST['form_build_id'] : FALSE;
    // Issue the client a new build_id, make sure that the form has it set
    // in the hidden field.
    if ($build_id_submit != $build_id) {
      $form_state['post']['form_build_id'] = $build_id;
    }
    // 6 hours cache life time for forms should be plenty.
    // @todo Provide UI for controlling the botcha_cache_expiration_timeout parameter.
    $expire = variable_get('botcha_cache_expiration_timeout', 21600);
    $data = array();
    $data['#cache_token'] = $this->getToken();
    // We use cache_form table.
    // Sneaky, but why build our own table since we are working side-by-side with form API?
    cache_set('botcha_' . $build_id, $data, 'cache_form', REQUEST_TIME + $expire);
  }

  function handle($mode, $form, $form_state) {
    parent::handle($mode, $form, $form_state);
    // !!~ @todo Reduce code duplication.
    // @see BotchaRecipe:applyRecipe
    $build_id = isset($_POST['form_build_id']) ? $_POST['form_build_id'] : $form['#build_id'];
    // Invalidate cache so resubmit will not work.
    // Make it to expire immediately.
    $expire = 0;
    $data = array();
    cache_set('botcha_' . $build_id, $data, 'cache_form', REQUEST_TIME + $expire);
  }
}


class BotchaRecipeUsingJsAbstract extends BotchaRecipe {

  function getInfo() {
    parent::getInfo();
    $this->error_text .= '<br />' . t('Please enable Javascript to use this form.');
  }

  // !!? @todo Should we do this with a private parameter instead of method?
  function getFieldCount() {
    return 1;
  }

  function generateFormElements() {
    $fields = $this->getProperty(
      $this->settings['fields'],
      'getFields'
    );
    $js = $this->getProperty(
      $this->settings['js'],
      'getJs'
    );
    $form_elements = array(
      $fields[0]['name'] => array(
        '#type' => 'textfield',
        '#title' => t('Enter your name'), // Leave the bot enough hints for it to guess it is a good name field
        '#default_value' => $fields[0]['default_value'], // Store part of secure_token
        '#description' => t('Your first name.'),
        // This is for human users without CSS.
        '#prefix' => '<div class="' . $fields[0]['class'] . '">'
            // [#1004218] Addition for screen readers. Apparently, they don't hande CSS display:none.
            . '<span class="description"> (' . t('If you\'re a human, don\'t change the following field') . ')</span>'
      ,
        // @todo Move it to constant since it is also used in error_text.
        '#suffix' => '</div>' . '<noscript>' . t('Please enable Javascript to use this form.') . '</noscript>',
        '#attributes' => array('class' => array($fields[0]['class']), 'autocomplete' => 'off'),
        '#weight' => -20,
        '!valid_token' => $js['secure_token'],
      ),
    );
    $js_value = $this->getProperty(
      $this->settings['js']['value'],
      'getJsValue'
    );
    if (!empty($js_value)) {
      drupal_add_js($js_value, array('type' => 'inline', 'preprocess' => FALSE));
    }
    return array_merge(parent::generateFormElements(), $form_elements);
  }

  function getField($delta) {
    return array_merge(parent::getField($delta), array(
      'default_value' => $this->getProperty(
        $this->settings['fields'][$delta]['default_value'],
        'getFieldDefault',
        $delta
      ),
    ));
  }

  /**
   * What server sends to JS in the field.
   *
   * @return string
   */
  function getFieldDefault($delta) {
    $fields = $this->getProperty(
      $this->settings['fields'],
      'getFields'
    );
    $js = $this->getProperty(
      $this->settings['js'],
      'getJs'
    );
    $field_prefix = $fields[$delta]['prefix'];
    $chops_positions = array_keys($js['chops']);
    $secure_token = $js['secure_token'];
    return $field_prefix . substr(
      $secure_token,
      $chops_positions[0],
      $chops_positions[1]
    );
  }

  public function getCss() {
    $fields = $this->getProperty(
      $this->settings['fields'],
      'getFields'
    );
    return 'div.' . $fields[0]['class'] . ' { display: none; visibility: hidden; }';
  }

  public function getJs() {
    // JS has to reconstruct the token from tok1, part of field_dflt, tok2.
    return array(
      'name' => $this->getProperty(
        $this->settings['js']['name'],
        'getJsName'
      ),
      'pos' => $this->getProperty(
        $this->settings['js']['pos'],
        'getJsPos'
      ),
      'match' => $this->getProperty(
        $this->settings['js']['match'],
        'getJsMatch'
      ),
      'secure_token' => $this->getProperty(
        $this->settings['js']['secure_token'],
        'getJsSecureToken'
      ),
      'chops' => $this->getProperty(
        $this->settings['js']['chops'],
        'getJsChops'
      ),
      // Value will be get just before adding js to the page.
      // It is to avoid endless cycles.
      //'value' => $this->getProperty(
      //  $this->settings['js']['value'],
      //  'getJsValue'
      //),
    );
  }

  /**
   * Get name of JS-script, attached to field.
   */
  function getJsName() {
    // Must start with a literal.
    return 'a' . substr($this->secret, 0, 10) . substr(
      $this->getProperty(
        $this->seed,
        'getSeed'
      ),
      6,
      8
    );
  }

  /**
   * What position is the part of the token.
   */
  function getJsPos() {
    return strlen($this->getProperty(
      $this->settings['fields'][0]['prefix'],
      'getFieldPrefix'
    ));
  }

  /**
   *  What JS matches in the field.
   */
  function getJsMatch() {
    $chop_positions = array_keys($this->getProperty(
      $this->settings['js']['chops'],
      'getJsChops'
    ));
    return substr(
      $this->getFieldDefault(0),
      0,
      $this->getJsPos() + mt_rand(2, $chop_positions[1])
    );
  }

  function getJsChops() {
    $secure_token = $this->getProperty(
      $this->settings['js']['secure_token'],
      'getJsSecureToken'
    );
    // Chop the token in 3 parts.
    $js_chops = array();
    $chop1 = 2;
    $js_chops[$chop1] = substr($secure_token, 0, $chop1);
    $chop2 = mt_rand(5, 8);
    $js_chops[$chop2] = substr($secure_token, $chop1 + $chop2);
    return $js_chops;
  }

  function getJsSecureToken() {
    return substr(
      $this->getProperty(
        $this->seed,
        'getSeed'
      ),
      4,
      -2
    ) . '_form';
  }

  function getJsValue() {}
}


class BotchaRecipeHoneypot extends BotchaRecipeUsingJsAbstract {

  function getInfo() {
    parent::getInfo();
    $this->description = t('Insert JS+CSS+honeypot field.')
      . ' ' . t('Bots will not run JS or will mess with the field')
      . ' ' . t('%parts are added to the form.', array('%parts' => t('Honeypot field') . ', CSS , JS'))
      . ' ' . t('CSS hides the input field.')
      . ' ' . t('JS enters key value into the field.');
  }

  function getJsValue() {
    // Just ensure that parameters are set.
    $fields = $this->getProperty(
      $this->settings['fields'],
      'getFields'
    );
    $js = $this->getProperty(
      $this->settings['js'],
      'getJs'
    );
    $js_tok1 = reset($js['chops']);
    $js_tok2 = next($js['chops']);
    return <<<END
(function ($) {
  Drupal.behaviors.{$js['name']} = {
    attach: function (context, settings) {
      $("input.{$fields[0]['class']}").each(function() {
        f=$(this)[0];
        if (f.value.indexOf("{$js['match']}")==0){f.value="{$js_tok1}"+f.value.substring({$js['pos']})+"{$js_tok2}";}
      });
    }
  };
})(jQuery);
END;
  }

  function isSpam($form, $form_state) {
    $isSpam = parent::isSpam($form, $form_state);
    foreach ($this->generateFormElements() as $field_name => $formElement) {
      if (isset($formElement['!valid_token']) && isset($form_state['botcha_submit_values'][$field_name])
        && $form_state['botcha_submit_values'][$field_name] !== $formElement['!valid_token']) {
        $isSpam = TRUE;
        break;
      }
    }
    return $isSpam;
  }
}


// @todo BotchaRecipeHoneypot2 Refactor it as a configuration of BotchaRecipeHoneypot.
class BotchaRecipeHoneypot2 extends BotchaRecipeHoneypot {

  function getFieldName($delta) {
    switch ($delta) {
      case 0:
      default:
        return parent::getFieldName($delta);
        break;
    }
  }

  function getCss() {
    $fields = $this->getProperty(
      $this->settings['fields'],
      'getFields'
    );
    $js = $this->getProperty(
      $this->settings['js'],
      'getJs'
    );
    $chops_positions = array_keys($js['chops']);
    $css_tok2 = substr($js['secure_token'], $chops_positions[0] + $chops_positions[1]);
    // 'a' before css token is because font name must start with a letter.
    return parent::getCss() . " input.{$fields[0]['class']} { font-family: sans-serif, a$css_tok2 !important; }";
  }

  function getJsValue() {
    $fields = $this->getProperty(
      $this->settings['fields'],
      'getFields'
    );
    $js = $this->getProperty(
      $this->settings['js'],
      'getJs'
    );
    $js_tok1 = reset($js['chops']);
    $selector = "input.{$fields[0]['class']}";
    return <<<END
(function ($) {
  Drupal.behaviors.{$js['name']} = {
    attach: function (context, settings) {
      $("{$selector}").each(function() {
        f=$(this)[0];
        tok2=$(f).css('fontFamily').split(',')[1].trim();
        if(tok2.charAt(0) == "'" || tok2.charAt(0) == '"') tok2=tok2.substring(1, tok2.length-1);
        tok2=tok2.substring(1, tok2.length);
        if (f.value.indexOf("{$js['match']}")==0){f.value="{$js_tok1}"+f.value.substring({$js['pos']})+tok2;}
      });
    }
  };
}(jQuery));
END;
  }
}


class BotchaRecipeObscureUrl extends BotchaRecipeUsingJsAbstract {

  function getInfo() {
    parent::getInfo();
    $this->description = t('Insert a new field into form action URL.')
      . ' ' . t('Bots will not run JS and miss the field.')
      . ' ' . t('%parts is added to the form.', array('%parts' => 'JS'))
      . ' ' . t('JS enters key value into the field.');
  }

  function isSpam($form, $form_state) {
    $isSpam = parent::isSpam($form, $form_state);
    foreach ($this->url_elements as $field => $value) {
      $url_field = isset($_GET[$field]) ? $_GET[$field] : FALSE;
      unset($_GET[$field]);
      if (isset($value['!valid_token']) && $url_field !== $value['!valid_token']) {
        $isSpam = TRUE;
        break;
      }
    }
    return $isSpam;
  }

  function applyRecipe(&$form, &$form_state) {
    // Make some preparations dependent on form before applying.
    $this->settings['form_id'] = $form['#id'];
    $this->settings['form_action'] = $form['#action'];
    parent::applyRecipe($form, $form_state);
    // !!~ @todo Merge url_elements with just fields.
    $this->url_elements[$this->settings['fields'][2]['name']] = array(
      // Describe URL field. JS will return token in URL field.
      '#type' => 'textfield',
      '#default_value' => '',
      '!valid_token' => $this->settings['js']['secure_token'],
    );
  }

  function getFieldCount() {
    return 3;
  }

  function getFieldName($delta) {
    switch ($delta) {
      case 2:
        return substr(
          $this->getProperty(
            $this->seed,
            'getSeed'
          ),
          1,
          4
        ) . '_name';
        break;
      case 1:
        return parent::getFieldName($delta);
        break;
      case 0:
      default:
        // @todo BotchaRecipeObscureUrl getFieldName Remove hardcode.
        return 'botcha';
        break;
    }
  }

  function getJsValue() {
    // Just ensure that parameters are set.
    $fields = $this->getProperty(
      $this->settings['fields'],
      'getFields'
    );
    $js = $this->getProperty(
      $this->settings['js'],
      'getJs'
    );
    $chops_positions = array_keys($js['chops']);
    $field2_name = $fields[2]['name'];
    $form_id = str_replace('_', '-', $this->settings['form_id']);
    $submit = _botcha_url(
      $this->settings['form_action'],
      array('query' => array($field2_name => '__replace__'))
    );
    // Secure_token.
    $submit = preg_replace(
      '/__replace__/',
      $js['chops'][$chops_positions[0]] . '\'+v+\'' . $js['chops'][$chops_positions[1]],
      $submit
    );
    return <<<END
(function ($) {
  Drupal.behaviors.{$js['name']} = {
    attach: function (context, settings) {
      $("input.{$fields[1]['class']}").each(function() {
        f=$(this)[0];
        if (f.value.indexOf("{$js['match']}")==0){
          v=f.value.substring({$js['pos']});
          $("#{$form_id}").get(0).setAttribute('action', '{$submit}');
        }
      });
    }
  };
}(jQuery));
END;
  }

  function generateFormElements() {
    $fields = $this->getProperty(
      $this->settings['fields'],
      'getFields'
    );
    return array_merge(parent::generateFormElements(), array(
      // Add hidden field to keep part of the token.
      $fields[1]['name'] => array(
        '#type' => 'hidden',
        // Store part of secure_token.
        '#default_value' => $fields[1]['default_value'],
        '#attributes' => array('class' => $fields[1]['class']),
        '#weight' => 20,
      ),
/*
      // Describe URL field. JS will return token in URL field.
      $fields[1]['name'] => array(
        '#type' => 'textfield',
        '#default_value' => '',
        '!valid_token' => $this->settings['js']['secure_token'],
      ),
 *
 */
    ));
  }
}


class BotchaRecipeTimegate extends BotchaRecipe {

  protected $time;

  function getInfo() {
    parent::getInfo();
    $this->description = t('Check time spended for submitting a form.')
      . ' ' . t('Bots submit form too fast.')
      . ' ' . t('Form is marked with timestamp which is checked during submit.');
    $this->error_text .= '<br />' . t('Form is submitted too fast.')
      . '<br />' . t('Please spend more time filling in the form.');
  }

  // @todo Should we do this with a private parameter instead of method?
  function getFieldCount() {
    return 1;
  }

  function getFieldName($delta) {
    switch ($delta) {
      case 0:
      default:
        return 'timegate';
        break;
    }
  }

  function generateFormElements() {
    $fields = $this->getProperty(
      $this->settings['fields'],
      'getFields'
    );
    return array_merge(parent::generateFormElements(), array(
      $fields[0]['name'] => array(
        '#type' => 'hidden',
        '#title' => 'Timegate',
        '#weight' => 5,
        '#required' => FALSE,
        '#default_value' => time(),
      ),
    ));
  }
/* !!-{
  function applyRecipe(&$form, &$form_state) {
    // Save the timestamp of form creation
    // to compare it with time() during validation.
    switch ($this->method) {
      case 'build_id':
        $this->time = time();
        break;
      case 'build_id_submit':
        $this->time = $form_state['input']['timegate'];
        break;
    }
    parent::applyRecipe($form, $form_state);
  }
 * !!-}
 */

  function isSpam($form, $form_state) {
    $isSpam = parent::isSpam($form, $form_state);
    // Timegate method validation.
    $absence = empty($form['timegate'])
      || empty($form_state['values']['timegate'])
      || empty($form_state['post']['timegate']);
    $minimal_delay = variable_get('botcha_timegate', 8);
    $form_generated = $form_state['values']['timegate'];
    $form_submitted = $form_state['post']['timegate'];
    if ($absence || ((int)$form_submitted < ((int)$form_generated + (int)$minimal_delay))) {
      $isSpam = TRUE;
    }
    return $isSpam;
  }
}
